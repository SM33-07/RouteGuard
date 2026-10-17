import React, { useEffect, useRef, useState, useCallback } from 'react';
import {
  Map as MapLibreMap,
  LngLatBounds,
  Marker,
  NavigationControl,
  AttributionControl,
  type GeoJSONSource,
  type ErrorEvent,
} from 'maplibre-gl';
import type { Feature, FeatureCollection, LineString } from 'geojson';
import 'maplibre-gl/dist/maplibre-gl.css';
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
  onOpenSchematic: () => void;
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
  onOpenSchematic,
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

    // Vector tile style from OpenFreeMap
    const styleUrl = 'https://tiles.openfreemap.org/styles/liberty';

    const map = new MapLibreMap({
      container: mapContainerRef.current,
      style: styleUrl,
      center: MUJ_CAMPUS_CENTER,
      zoom: MUJ_DEFAULT_ZOOM,
      pitch: 25,
      bearing: -15,
      attributionControl: false, // Customized below for compliant styling
    });

    map.addControl(
      new AttributionControl({
        compact: false,
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
      // Catch tile loading or connectivity failures
      if (e && e.error && e.error.message && e.error.message.includes('fetch')) {
        setMapError('Vector tile connection unavailable. Remote basemap requires network access.');
      }
    });

    mapRef.current = map;

    // Responsive container ResizeObserver
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

  // 2. Render GeoJSON Walkway Layers (active, closed, and minimum-hop route)
  const updateGeoJSONLayers = useCallback(() => {
    const map = mapRef.current;
    if (!map || !mapLoaded || !graph) return;

    // Build map of vertex coordinates
    const coordMap = new Map<number, [number, number]>();
    CAMPUS_VENUES.forEach((v) => {
      coordMap.set(v.id, v.coordinates);
    });

    // Build FeatureCollection for edges
    const edgeFeatures: Feature<LineString>[] = [];
    const routeFeatures: Feature<LineString>[] = [];

    graph.edges.forEach((edge) => {
      const uCoords = coordMap.get(edge.u);
      const vCoords = coordMap.get(edge.v);
      if (!uCoords || !vCoords) return;

      const isRoute = routeEdgePairs.has(`${edge.u}-${edge.v}`);

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
          coordinates: [uCoords, vCoords],
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
      map.addSource('campus-edges', {
        type: 'geojson',
        data: edgesGeoJSON,
      });

      // Layer 1: Modeled walkways (Active)
      map.addLayer({
        id: 'campus-edges-active',
        type: 'line',
        source: 'campus-edges',
        filter: ['all', ['==', ['get', 'active'], true], ['!=', ['get', 'isRoute'], true]],
        layout: {
          'line-join': 'round',
          'line-cap': 'round',
        },
        paint: {
          'line-color': theme === 'light' ? '#334155' : theme === 'high-contrast' ? '#ffffff' : '#64748b',
          'line-width': ['interpolate', ['linear'], ['zoom'], 15, 3, 18, 5],
          'line-opacity': 0.85,
        },
      });

      // Layer 2: Closed walkways (Dashed red)
      map.addLayer({
        id: 'campus-edges-closed',
        type: 'line',
        source: 'campus-edges',
        filter: ['==', ['get', 'active'], false],
        layout: {
          'line-join': 'round',
          'line-cap': 'round',
        },
        paint: {
          'line-color': '#ef4444',
          'line-width': ['interpolate', ['linear'], ['zoom'], 15, 3.5, 18, 5.5],
          'line-dasharray': [2, 2],
          'line-opacity': 0.95,
        },
      });

      // Interactive hover & click on edges
      map.on('mouseenter', 'campus-edges-active', () => {
        map.getCanvas().style.cursor = 'pointer';
      });
      map.on('mouseleave', 'campus-edges-active', () => {
        map.getCanvas().style.cursor = '';
      });
      map.on('mouseenter', 'campus-edges-closed', () => {
        map.getCanvas().style.cursor = 'pointer';
      });
      map.on('mouseleave', 'campus-edges-closed', () => {
        map.getCanvas().style.cursor = '';
      });

      map.on('click', 'campus-edges-active', (e) => {
        if (!e.features || !e.features[0]) return;
        const p = e.features[0].properties;
        setSelectedEdgeForAction({
          id: Number(p.edgeId),
          u: Number(p.u),
          v: Number(p.v),
          active: Boolean(p.active),
        });
      });

      map.on('click', 'campus-edges-closed', (e) => {
        if (!e.features || !e.features[0]) return;
        const p = e.features[0].properties;
        setSelectedEdgeForAction({
          id: Number(p.edgeId),
          u: Number(p.u),
          v: Number(p.v),
          active: Boolean(p.active),
        });
      });
    }

    // Route Source & Layer
    const routeSource = map.getSource('campus-route') as GeoJSONSource;
    if (routeSource) {
      routeSource.setData(routeGeoJSON);
    } else {
      map.addSource('campus-route', {
        type: 'geojson',
        data: routeGeoJSON,
      });

      // Layer 3: Glowing Route Background
      map.addLayer({
        id: 'campus-route-glow',
        type: 'line',
        source: 'campus-route',
        layout: {
          'line-join': 'round',
          'line-cap': 'round',
        },
        paint: {
          'line-color': '#06b6d4',
          'line-width': ['interpolate', ['linear'], ['zoom'], 15, 8, 18, 14],
          'line-opacity': 0.45,
          'line-blur': 3,
        },
      });

      // Layer 4: Primary Minimum-Hop Route Line
      map.addLayer({
        id: 'campus-route-line',
        type: 'line',
        source: 'campus-route',
        layout: {
          'line-join': 'round',
          'line-cap': 'round',
        },
        paint: {
          'line-color': '#38bdf8',
          'line-width': ['interpolate', ['linear'], ['zoom'], 15, 4.5, 18, 7.5],
          'line-opacity': 1.0,
        },
      });
    }
  }, [mapLoaded, graph, routeEdgePairs, theme]);

  useEffect(() => {
    updateGeoJSONLayers();
  }, [updateGeoJSONLayers]);

  // 3. Render HTML Venue Markers on Map
  useEffect(() => {
    const map = mapRef.current;
    if (!map || !mapLoaded || !graph) return;

    // Clear existing markers
    markersRef.current.forEach((marker) => marker.remove());
    markersRef.current.clear();

    CAMPUS_VENUES.forEach((venue) => {
      const isSrc = venue.id === srcVertex;
      const isDst = venue.id === dstVertex;
      const isSelected = venue.id === selectedVertexId;
      const isReached = reachedSet.has(venue.id);
      const isUnreached = !isReached && traversalResult !== null;

      // Create custom marker DOM element
      const el = document.createElement('div');
      el.className = `map-venue-marker ${isSrc ? 'marker-src' : ''} ${isDst ? 'marker-dst' : ''} ${
        isSelected ? 'marker-selected' : ''
      } ${isUnreached ? 'marker-unreached' : ''}`;
      el.setAttribute('role', 'button');
      el.setAttribute('tabindex', '0');
      el.setAttribute(
        'aria-label',
        `Venue ${venue.id}: ${venue.label}${isSrc ? ' (Source)' : ''}${isDst ? ' (Destination)' : ''}`
      );

      // Icon symbol based on category or state
      let badgeHtml = '';
      if (isSrc) {
        badgeHtml = '<span class="marker-badge src-badge">SRC</span>';
      } else if (isDst) {
        badgeHtml = '<span class="marker-badge dst-badge">DST</span>';
      } else if (isUnreached) {
        badgeHtml = '<span class="marker-badge unreached-badge">UNREACHED</span>';
      } else if (venue.landmarkIndex) {
        badgeHtml = `<span class="marker-badge landmark-badge">#${venue.landmarkIndex}</span>`;
      }

      const iconEmoji =
        venue.category === 'library'
          ? '🏛️'
          : venue.category === 'dining'
          ? '🍽️'
          : venue.category === 'sports'
          ? '⚽'
          : venue.category === 'arrival'
          ? '🚪'
          : '🏢';

      el.innerHTML = `
        <div class="marker-pin">
          <span class="marker-icon">${iconEmoji}</span>
          <span class="marker-id">${venue.id}</span>
        </div>
        <div class="marker-label-tag">
          ${venue.label}
          ${badgeHtml}
        </div>
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
  }, [mapLoaded, graph, srcVertex, dstVertex, selectedVertexId, reachedSet, traversalResult, onSelectVertex, onSetSource, onSetDestination]);

  // Camera focus on selected vertex (e.g. from Venue Index or Search)
  useEffect(() => {
    const map = mapRef.current;
    if (!map || !mapLoaded || selectedVertexId === null) return;
    const venue = CAMPUS_VENUES.find((v) => v.id === selectedVertexId);
    if (venue) {
      map.easeTo({
        center: venue.coordinates,
        zoom: Math.max(map.getZoom(), 16.6),
        duration: 650,
      });
    }
  }, [selectedVertexId, mapLoaded]);

  // Fit bounds to entire campus network
  const handleFitNetwork = () => {
    const map = mapRef.current;
    if (!map) return;
    const bounds = new LngLatBounds();
    CAMPUS_VENUES.forEach((v) => bounds.extend(v.coordinates));
    map.fitBounds(bounds, { padding: 60, maxZoom: 17.5, duration: 800 });
  };

  // Reset to default center
  const handleResetCenter = () => {
    const map = mapRef.current;
    if (!map) return;
    map.flyTo({ center: MUJ_CAMPUS_CENTER, zoom: MUJ_DEFAULT_ZOOM, pitch: 25, bearing: -15, duration: 700 });
  };

  return (
    <div className="campus-map-wrapper">
      {/* Top Map Action Bar */}
      <div className="map-floating-toolbar">
        <div className="toolbar-left">
          <span className="map-badge">
            <span className="status-indicator-dot online"></span>
            <span>MapLibre GL Vector Basemap</span>
          </span>
          <button
            type="button"
            className="btn btn-sm btn-toolbar"
            onClick={handleFitNetwork}
            title="Fit all campus venues in view"
            id="btn-fit-network"
          >
            ⤢ Fit Campus Network
          </button>
          <button
            type="button"
            className="btn btn-sm btn-toolbar"
            onClick={handleResetCenter}
            title="Reset map view to campus center"
            id="btn-reset-map-view"
          >
            ⟳ Reset View
          </button>
        </div>

        <div className="toolbar-right">
          <button
            type="button"
            className="btn btn-sm btn-schematic-toggle"
            onClick={onOpenSchematic}
            id="btn-open-schematic-ref"
            title="View original cream schematic drawing & satellite images"
          >
            📋 Schematic &amp; Satellites Reference
          </button>
        </div>
      </div>

      {/* Mandatory Presentation Disclaimer */}
      <div className="campus-model-disclaimer" id="campus-disclaimer">
        <span style={{ fontSize: '13px' }}>📌</span>
        <span>
          <strong>Illustrative graph model:</strong> modeled connections are simplified for DSA demonstration and are not verified pedestrian directions.
        </span>
      </div>

      {/* Network / Tile Error Fallback Notice */}
      {mapError && (
        <div className="map-error-banner" role="alert">
          <span>⚠️ {mapError}</span>
          <button
            className="btn btn-sm btn-secondary"
            onClick={onOpenSchematic}
            style={{ marginLeft: '12px' }}
          >
            Switch to Offline Schematic View
          </button>
        </div>
      )}

      {/* MapLibre WebGL Canvas Container */}
      <div ref={mapContainerRef} className="maplibre-container" id="maplibre-campus-canvas" />

      {/* Floating Map Legend */}
      <div className="map-floating-legend">
        <div className="legend-title">Campus Graph Legend</div>
        <div className="legend-items">
          <div className="legend-item">
            <span className="legend-line active-line"></span>
            <span>Active Walkway</span>
          </div>
          <div className="legend-item">
            <span className="legend-line closed-line"></span>
            <span>Closed / Blocked (Dashed)</span>
          </div>
          <div className="legend-item">
            <span className="legend-line route-line"></span>
            <span>Minimum-Hop Route</span>
          </div>
          <div className="legend-item">
            <span className="legend-dot src-dot"></span>
            <span>Source [SRC]</span>
          </div>
          <div className="legend-item">
            <span className="legend-dot dst-dot"></span>
            <span>Destination [DST]</span>
          </div>
          <div className="legend-item">
            <span className="legend-dot unreached-dot"></span>
            <span>Unreached Partition</span>
          </div>
        </div>
      </div>

      {/* Explicit Walkway Action Modal / Popover */}
      {selectedEdgeForAction && (
        <div className="edge-action-modal-backdrop" onClick={() => setSelectedEdgeForAction(null)}>
          <div className="edge-action-card" onClick={(e) => e.stopPropagation()}>
            <div className="edge-action-header">
              <span style={{ fontWeight: 700, color: 'var(--cyan)' }}>
                Walkway e{selectedEdgeForAction.id}
              </span>
              <button
                className="btn btn-sm btn-icon"
                onClick={() => setSelectedEdgeForAction(null)}
                aria-label="Close"
              >
                ✕
              </button>
            </div>
            <div className="edge-action-body">
              <p style={{ margin: '0 0 10px 0', fontSize: '13px' }}>
                Connecting:{' '}
                <strong>
                  Node {selectedEdgeForAction.u} ({CAMPUS_VENUES[selectedEdgeForAction.u]?.label})
                </strong>{' '}
                ↔{' '}
                <strong>
                  Node {selectedEdgeForAction.v} ({CAMPUS_VENUES[selectedEdgeForAction.v]?.label})
                </strong>
              </p>
              <p style={{ margin: '0 0 14px 0', fontSize: '12px', color: 'var(--text-secondary)' }}>
                Status:{' '}
                <span
                  className={`status-pill ${selectedEdgeForAction.active ? 'success' : 'error'}`}
                >
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
                  {selectedEdgeForAction.active ? '🚫 Close / Disable Walkway' : '✅ Re-Open Walkway'}
                </button>
              </div>
            </div>
          </div>
        </div>
      )}
    </div>
  );
};
