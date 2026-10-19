import React, { useEffect, useRef, useState, useCallback } from 'react';
import {
  Map as MapLibreMap,
  LngLatBounds,
  Marker,
  NavigationControl,
  AttributionControl,
  setWorkerUrl,
  type GeoJSONSource,
  type ErrorEvent,
} from 'maplibre-gl';
import type { Feature, FeatureCollection, LineString } from 'geojson';
import 'maplibre-gl/dist/maplibre-gl.css';
// Fix for Vite bundler: explicitly bundle the MapLibre worker so tiles render correctly
import maplibreWorkerUrl from 'maplibre-gl/dist/maplibre-gl-worker.mjs?worker&url';
setWorkerUrl(maplibreWorkerUrl);
import { CAMPUS_VENUES, MUJ_CAMPUS_CENTER, MUJ_DEFAULT_ZOOM } from '../campus_venues';
import type { GraphData, RouteResult, TraversalResult } from '../wasm/bridge';

interface CampusMapProps {
  graph: GraphData | null;
  srcVertex: number;
  dstVertex: number;
  selectedVertexId: number | null;
  routeResult: RouteResult | null;
  traversalResult: TraversalResult | null;
  theme: 'dark' | 'light' | 'high-contrast';
  onSelectVertex: (vId: number) => void;
  onSetSource: (vId: number) => void;
  onSetDestination: (vId: number) => void;
  onToggleEdge: (edgeId: number) => void;
}

export const CampusMap: React.FC<CampusMapProps> = ({
  graph,
  srcVertex,
  dstVertex,
  selectedVertexId,
  routeResult,
  traversalResult,
  theme,
  onSelectVertex,
  onSetSource,
  onSetDestination,
  onToggleEdge,
}) => {
  const mapContainerRef = useRef<HTMLDivElement>(null);
  const mapRef = useRef<MapLibreMap | null>(null);
  const markersRef = useRef<Map<number, Marker>>(new Map());

  const [mapLoaded, setMapLoaded] = useState(false);
  const [mapError, setMapError] = useState<string | null>(null);
  const [selectedEdgeForAction, setSelectedEdgeForAction] = useState<{
    id: number;
    u: number;
    v: number;
    active: boolean;
  } | null>(null);

  // Set of reached vertices from current traversal
  const reachedSet = React.useMemo(() => {
    const s = new Set<number>();
    if (traversalResult && traversalResult.order) {
      traversalResult.order.forEach((id) => s.add(id));
    }
    return s;
  }, [traversalResult]);

  // Set of vertices on the highlighted route path
  const routeVertexSet = React.useMemo(() => {
    const s = new Set<number>();
    if (routeResult && routeResult.found) {
      routeResult.path.forEach((id) => s.add(id));
    }
    return s;
  }, [routeResult]);

  // Set of route edges for highlighting
  const routeEdgePairs = React.useMemo(() => {
    const pairs = new Set<string>();
    if (!routeResult || !routeResult.found || routeResult.path.length < 2) return pairs;
    for (let i = 0; i < routeResult.path.length - 1; i++) {
      const u = routeResult.path[i];
      const v = routeResult.path[i + 1];
      pairs.add(`${u}-${v}`);
      pairs.add(`${v}-${u}`);
    }
    return pairs;
  }, [routeResult]);

  // 1. Initialize MapLibre Map instance
  useEffect(() => {
    if (!mapContainerRef.current) return;

    const styleUrl = 'https://tiles.openfreemap.org/styles/liberty';

    const map = new MapLibreMap({
      container: mapContainerRef.current,
      style: styleUrl,
      center: MUJ_CAMPUS_CENTER,
      zoom: MUJ_DEFAULT_ZOOM,
      pitch: 20,
      bearing: -10,
      attributionControl: false,
    });

    map.addControl(
      new AttributionControl({
        compact: true,
        customAttribution:
          '<a href="https://openfreemap.org" target="_blank" rel="noopener">OpenFreeMap</a> © <a href="https://www.openstreetmap.org/copyright" target="_blank" rel="noopener">OpenStreetMap</a> contributors',
      }),
      'bottom-right'
    );

    map.addControl(new NavigationControl({ visualizePitch: true }), 'top-right');

    map.on('load', () => {
      setMapLoaded(true);
      setMapError(null);
    });

    map.on('error', (e: ErrorEvent) => {
      if (e && e.error && e.error.message && e.error.message.includes('fetch')) {
        setMapError('Vector tile connection unavailable. Remote basemap requires network access.');
      }
    });

    mapRef.current = map;

    // Responsive ResizeObserver
    const ro = new ResizeObserver(() => {
      map.resize();
    });
    ro.observe(mapContainerRef.current);

    return () => {
      ro.disconnect();
      map.remove();
      mapRef.current = null;
    };
  }, []);

  // 2. Render GeoJSON Walkway Layers
  const updateGeoJSONLayers = useCallback(() => {
    const map = mapRef.current;
    if (!map || !mapLoaded || !graph) return;

    const coordMap = new Map<number, [number, number]>();
    CAMPUS_VENUES.forEach((v) => {
      coordMap.set(v.id, v.coordinates);
    });

    // Display-only visual polylines for edges where a straight line would cut through building footprints.
    // Purely display geometry: does NOT alter C graph topology, hops, or algorithm results.
    const DISPLAY_EDGE_GEOMETRIES: Record<string, [number, number][]> = {
      // e17 (0 <-> 3): Follows the western campus entrance corridor around building footprints
      '0-3': [
        [75.56386, 26.84158],
        [75.56402, 26.84218],
        [75.56435, 26.84275],
      ],
      '3-0': [
        [75.56435, 26.84275],
        [75.56402, 26.84218],
        [75.56386, 26.84158],
      ],
    };

    const edgeFeatures: Feature<LineString>[] = [];
    const routeFeatures: Feature<LineString>[] = [];

    graph.edges.forEach((edge) => {
      const uCoords = coordMap.get(edge.u);
      const vCoords = coordMap.get(edge.v);
      if (!uCoords || !vCoords) return;

      const isRoute = routeEdgePairs.has(`${edge.u}-${edge.v}`);
      const customCoords = DISPLAY_EDGE_GEOMETRIES[`${edge.u}-${edge.v}`];

      const feature: Feature<LineString> = {
        type: 'Feature',
        id: edge.id,
        properties: {
          edgeId: edge.id,
          u: edge.u,
          v: edge.v,
          active: edge.active,
          isRoute,
        },
        geometry: {
          type: 'LineString',
          coordinates: customCoords || [uCoords, vCoords],
        },
      };

      if (isRoute && edge.active) {
        routeFeatures.push(feature);
      }
      edgeFeatures.push(feature);
    });

    const edgesGeoJSON: FeatureCollection<LineString> = {
      type: 'FeatureCollection',
      features: edgeFeatures,
    };

    const routeGeoJSON: FeatureCollection<LineString> = {
      type: 'FeatureCollection',
      features: routeFeatures,
    };

    // Edge Source
    const edgeSource = map.getSource('campus-edges') as GeoJSONSource;
    if (edgeSource) {
      edgeSource.setData(edgesGeoJSON);
    } else {
      map.addSource('campus-edges', { type: 'geojson', data: edgesGeoJSON });

      map.addLayer({
        id: 'campus-edges-active',
        type: 'line',
        source: 'campus-edges',
        filter: ['all', ['==', ['get', 'active'], true], ['!=', ['get', 'isRoute'], true]],
        layout: { 'line-join': 'round', 'line-cap': 'round' },
        paint: {
          'line-color': theme === 'light' ? '#475569' : theme === 'high-contrast' ? '#ffffff' : '#7c93ae',
          'line-width': ['interpolate', ['linear'], ['zoom'], 15, 2.5, 18, 4.5],
          'line-opacity': 0.8,
        },
      });

      map.addLayer({
        id: 'campus-edges-closed',
        type: 'line',
        source: 'campus-edges',
        filter: ['==', ['get', 'active'], false],
        layout: { 'line-join': 'round', 'line-cap': 'round' },
        paint: {
          'line-color': '#ef4444',
          'line-width': ['interpolate', ['linear'], ['zoom'], 15, 3, 18, 5],
          'line-dasharray': [3, 2],
          'line-opacity': 0.95,
        },
      });

      map.on('mouseenter', 'campus-edges-active', () => { map.getCanvas().style.cursor = 'pointer'; });
      map.on('mouseleave', 'campus-edges-active', () => { map.getCanvas().style.cursor = ''; });
      map.on('mouseenter', 'campus-edges-closed', () => { map.getCanvas().style.cursor = 'pointer'; });
      map.on('mouseleave', 'campus-edges-closed', () => { map.getCanvas().style.cursor = ''; });

      map.on('click', 'campus-edges-active', (e) => {
        if (!e.features || !e.features[0]) return;
        const p = e.features[0].properties;
        setSelectedEdgeForAction({ id: Number(p.edgeId), u: Number(p.u), v: Number(p.v), active: Boolean(p.active) });
      });

      map.on('click', 'campus-edges-closed', (e) => {
        if (!e.features || !e.features[0]) return;
        const p = e.features[0].properties;
        setSelectedEdgeForAction({ id: Number(p.edgeId), u: Number(p.u), v: Number(p.v), active: Boolean(p.active) });
      });
    }

    // Route Source & Layer
    const routeSource = map.getSource('campus-route') as GeoJSONSource;
    if (routeSource) {
      routeSource.setData(routeGeoJSON);
    } else {
      map.addSource('campus-route', { type: 'geojson', data: routeGeoJSON });

      map.addLayer({
        id: 'campus-route-glow',
        type: 'line',
        source: 'campus-route',
        layout: { 'line-join': 'round', 'line-cap': 'round' },
        paint: {
          'line-color': '#06b6d4',
          'line-width': ['interpolate', ['linear'], ['zoom'], 15, 10, 18, 18],
          'line-opacity': 0.35,
          'line-blur': 4,
        },
      });

      map.addLayer({
        id: 'campus-route-line',
        type: 'line',
        source: 'campus-route',
        layout: { 'line-join': 'round', 'line-cap': 'round' },
        paint: {
          'line-color': '#38bdf8',
          'line-width': ['interpolate', ['linear'], ['zoom'], 15, 4, 18, 7],
          'line-opacity': 1.0,
        },
      });
    }
  }, [mapLoaded, graph, routeEdgePairs, theme]);

  useEffect(() => {
    updateGeoJSONLayers();
  }, [updateGeoJSONLayers]);

  // 3. Render HTML Venue Markers — smart label visibility
  useEffect(() => {
    const map = mapRef.current;
    if (!map || !mapLoaded || !graph) return;

    markersRef.current.forEach((marker) => marker.remove());
    markersRef.current.clear();

    CAMPUS_VENUES.forEach((venue) => {
      const isSrc = venue.id === srcVertex;
      const isDst = venue.id === dstVertex;
      const isSelected = venue.id === selectedVertexId;
      const isOnRoute = routeVertexSet.has(venue.id);
      const isReached = reachedSet.has(venue.id);
      const isUnreached = !isReached && traversalResult !== null;

      // Label always visible for: SRC, DST, selected, on route
      const alwaysShowLabel = isSrc || isDst || isSelected || isOnRoute;

      const el = document.createElement('div');
      el.className = [
        'map-venue-marker',
        isSrc ? 'marker-src' : '',
        isDst ? 'marker-dst' : '',
        isSelected ? 'marker-selected' : '',
        isUnreached ? 'marker-unreached' : '',
        alwaysShowLabel ? 'label-always' : 'label-hover',
      ].filter(Boolean).join(' ');

      el.setAttribute('role', 'button');
      el.setAttribute('tabindex', '0');
      el.setAttribute(
        'aria-label',
        `Venue ${venue.id}: ${venue.label}${isSrc ? ' (Source)' : ''}${isDst ? ' (Destination)' : ''}`
      );

      let badgeHtml = '';
      if (isSrc)         badgeHtml = '<span class="marker-badge src-badge">SRC</span>';
      else if (isDst)    badgeHtml = '<span class="marker-badge dst-badge">DST</span>';
      else if (isUnreached) badgeHtml = '<span class="marker-badge unreached-badge">✗</span>';

      const iconEmoji =
        venue.category === 'library'  ? '🏛️'
        : venue.category === 'dining' ? '🍽️'
        : venue.category === 'sports' ? '⚽'
        : venue.category === 'arrival'? '🚪'
        : '🏢';

      el.innerHTML = `
        <div class="marker-pin">
          <span class="marker-icon">${iconEmoji}</span>
          <span class="marker-id">${venue.id}</span>
          ${badgeHtml}
        </div>
        <div class="marker-label-tag">${venue.label}</div>
      `;

      el.addEventListener('click', (evt) => {
        evt.stopPropagation();
        onSelectVertex(venue.id);
      });

      el.addEventListener('contextmenu', (evt) => {
        evt.preventDefault();
        evt.stopPropagation();
        onSelectVertex(venue.id);
        onSetSource(venue.id);
      });

      el.addEventListener('dblclick', (evt) => {
        evt.preventDefault();
        evt.stopPropagation();
        onSelectVertex(venue.id);
        onSetDestination(venue.id);
      });

      const marker = new Marker({ element: el, anchor: 'bottom' })
        .setLngLat(venue.coordinates)
        .addTo(map);

      markersRef.current.set(venue.id, marker);
    });
  }, [mapLoaded, graph, srcVertex, dstVertex, selectedVertexId, routeVertexSet, reachedSet, traversalResult, onSelectVertex, onSetSource, onSetDestination]);

  // Camera focus on selected vertex
  useEffect(() => {
    const map = mapRef.current;
    if (!map || !mapLoaded || selectedVertexId === null) return;
    const venue = CAMPUS_VENUES.find((v) => v.id === selectedVertexId);
    if (venue) {
      map.easeTo({ center: venue.coordinates, zoom: Math.max(map.getZoom(), 16.6), duration: 550 });
    }
  }, [selectedVertexId, mapLoaded]);

  const handleFitNetwork = () => {
    const map = mapRef.current;
    if (!map) return;
    const bounds = new LngLatBounds();
    CAMPUS_VENUES.forEach((v) => bounds.extend(v.coordinates));
    map.fitBounds(bounds, { padding: { top: 80, bottom: 100, left: 40, right: 40 }, maxZoom: 17, duration: 800 });
  };

  const handleResetCenter = () => {
    const map = mapRef.current;
    if (!map) return;
    map.flyTo({ center: MUJ_CAMPUS_CENTER, zoom: MUJ_DEFAULT_ZOOM, pitch: 20, bearing: -10, duration: 700 });
  };

  const handleRetryMap = () => {
    if (!mapRef.current) return;
    setMapError(null);
    setMapLoaded(false);
    try {
      mapRef.current.setStyle('https://tiles.openfreemap.org/styles/liberty');
    } catch {
      setMapError('Basemap connection failed on retry.');
    }
  };

  // Compact route path display
  const routePathDisplay = React.useMemo(() => {
    if (!routeResult || !routeResult.found || !graph) return null;
    return routeResult.path.map((id) => graph.vertices[id]?.label ?? `#${id}`);
  }, [routeResult, graph]);

  return (
    <div className="campus-map-wrapper">
      {/* Compact floating top toolbar */}
      <div className="map-floating-toolbar">
        <div className="toolbar-left">
          <span className="map-badge">
            <span className="status-indicator-dot online" />
            <span>OpenFreeMap · OSM</span>
          </span>
          <button
            type="button"
            className="btn btn-sm btn-toolbar"
            onClick={handleFitNetwork}
            title="Fit all 14 campus venues in view"
            id="btn-fit-network"
          >
            ⤢ Fit Network
          </button>
          <button
            type="button"
            className="btn btn-sm btn-toolbar"
            onClick={handleResetCenter}
            title="Reset map to campus center"
            id="btn-reset-map-view"
          >
            ⟳ Reset View
          </button>
        </div>

        <div className="toolbar-right">
          <span className="campus-model-disclaimer-inline" id="campus-disclaimer">
            <span>📌</span>
            <span>Illustrative graph model — not verified walkways</span>
          </span>
        </div>
      </div>

      {/* Lightweight non-blocking loading state */}
      {!mapLoaded && !mapError && (
        <div className="map-loading-overlay" role="status" aria-live="polite">
          <div className="map-loading-spinner" aria-hidden="true" />
          <span className="map-loading-text">Loading campus map…</span>
        </div>
      )}

      {/* Network / Tile Error Fallback */}
      {mapError && (
        <div className="map-error-banner" role="alert">
          <span>⚠️ {mapError} — Graph overlay remains operational.</span>
          <button
            type="button"
            className="btn btn-sm btn-retry-map"
            onClick={handleRetryMap}
            title="Retry connecting to basemap tiles"
          >
            ⟳ Retry
          </button>
        </div>
      )}

      {/* MapLibre WebGL Canvas */}
      <div ref={mapContainerRef} className="maplibre-container" id="maplibre-campus-canvas" />

      {/* Single legend — bottom-right, compact */}
      <div className="map-legend-compact" aria-label="Campus Graph Legend">
        <div className="legend-compact-title">Graph Legend</div>
        <div className="legend-compact-row">
          <span className="lc-line lc-active" />
          <span>Active walkway</span>
        </div>
        <div className="legend-compact-row">
          <span className="lc-line lc-closed" />
          <span>Closed / blocked</span>
        </div>
        <div className="legend-compact-row">
          <span className="lc-line lc-route" />
          <span>Min-hop route</span>
        </div>
        <div className="legend-compact-row">
          <span className="lc-dot lc-src" />
          <span>Source [SRC]</span>
        </div>
        <div className="legend-compact-row">
          <span className="lc-dot lc-dst" />
          <span>Destination [DST]</span>
        </div>
      </div>

      {/* Compact route summary bar — bottom, wrapping chips */}
      <div className="route-compact-bar" id="route-compact-bar">
        {routeResult?.found && routePathDisplay ? (
          <>
            <span className="rcb-hop-pill">
              {routeResult.hop_count} Hops
            </span>
            <div className="rcb-path-chips">
              {routePathDisplay.map((label, idx) => (
                <React.Fragment key={`rcb-${idx}`}>
                  <span
                    className={`rcb-chip ${idx === 0 ? 'chip-src' : idx === routePathDisplay.length - 1 ? 'chip-dst' : ''}`}
                    onClick={() => {
                      const vId = routeResult.path[idx];
                      onSelectVertex(vId);
                    }}
                  >
                    {routeResult.path[idx]}: {label}
                  </span>
                  {idx < routePathDisplay.length - 1 && (
                    <span className="rcb-arrow">→</span>
                  )}
                </React.Fragment>
              ))}
            </div>
            <span className="rcb-repr-tag">
              {/* Repr shown in inspector */}
            </span>
          </>
        ) : (
          <span className="rcb-blocked">
            ✗ No active path from {CAMPUS_VENUES[srcVertex]?.label ?? srcVertex} to {CAMPUS_VENUES[dstVertex]?.label ?? dstVertex}
          </span>
        )}
      </div>

      {/* Edge Action Modal */}
      {selectedEdgeForAction && (
        <div className="edge-action-modal-backdrop" onClick={() => setSelectedEdgeForAction(null)}>
          <div className="edge-action-card" onClick={(e) => e.stopPropagation()}>
            <div className="edge-action-header">
              <span style={{ fontWeight: 700, color: 'var(--cyan)' }}>
                Walkway e{selectedEdgeForAction.id}
              </span>
              <button
                className="btn btn-sm btn-secondary"
                onClick={() => setSelectedEdgeForAction(null)}
                aria-label="Close"
              >
                ✕
              </button>
            </div>
            <div className="edge-action-body">
              <p style={{ margin: '0 0 10px 0', fontSize: '13px' }}>
                <strong>{CAMPUS_VENUES[selectedEdgeForAction.u]?.label ?? selectedEdgeForAction.u}</strong>
                {' ↔ '}
                <strong>{CAMPUS_VENUES[selectedEdgeForAction.v]?.label ?? selectedEdgeForAction.v}</strong>
              </p>
              <p style={{ margin: '0 0 14px 0', fontSize: '12px', color: 'var(--text-secondary)' }}>
                Status:{' '}
                <span className={`status-pill ${selectedEdgeForAction.active ? 'success' : 'error'}`}>
                  {selectedEdgeForAction.active ? 'ACTIVE' : 'CLOSED'}
                </span>
              </p>
              <div style={{ display: 'flex', gap: '8px', justifyContent: 'flex-end' }}>
                <button
                  type="button"
                  className="btn btn-sm btn-secondary"
                  onClick={() => setSelectedEdgeForAction(null)}
                >
                  Cancel
                </button>
                <button
                  type="button"
                  id="btn-confirm-toggle-edge"
                  className={`btn btn-sm ${selectedEdgeForAction.active ? 'btn-danger' : 'btn-success'}`}
                  onClick={() => {
                    onToggleEdge(selectedEdgeForAction.id);
                    setSelectedEdgeForAction(null);
                  }}
                >
                  {selectedEdgeForAction.active ? '🚫 Close Walkway' : '✅ Re-Open Walkway'}
                </button>
              </div>
            </div>
          </div>
        </div>
      )}
    </div>
  );
};
