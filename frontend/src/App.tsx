import React, { useEffect, useState, useMemo, useCallback } from 'react';
import {
  getRouteGuardBridge,
  RouteGuardBridge,
  type GraphData,
  type TraversalResult,
  type RouteResult,
  type ComponentsResult,
  type NetworkState,
  type FailureTableResult,
} from './wasm/bridge';
import { CampusMap } from './components/CampusMap';
import { CAMPUS_VENUES } from './campus_venues';

export const App: React.FC = () => {
  const [bridge, setBridge] = useState<RouteGuardBridge | null>(null);
  const [graph, setGraph] = useState<GraphData | null>(null);
  const [loading, setLoading] = useState<boolean>(true);
  const [errorMsg, setErrorMsg] = useState<string | null>(null);

  // Graph Mode: 'campus' (MUJ Campus Demo) | 'f1' (F1 Reference Graph)
  const [graphMode, setGraphMode] = useState<'campus' | 'f1'>('campus');

  // Controls & Selection
  const [srcVertex, setSrcVertex] = useState<number>(0);
  const [dstVertex, setDstVertex] = useState<number>(6); // Academic Block 3 in campus mode
  const [selectedVertexId, setSelectedVertexId] = useState<number | null>(0);
  const [useMatrix, setUseMatrix] = useState<boolean>(false);

  // Panel Collapsible States (for Map-Centric Operations Console)
  const [leftPanelOpen, setLeftPanelOpen] = useState<boolean>(true);
  const [rightPanelOpen, setRightPanelOpen] = useState<boolean>(true);

  // Inspector Tab: 'venue' | 'route' | 'network' | 'venueIndex' | 'schematic'
  const [inspectorTab, setInspectorTab] = useState<'venue' | 'route' | 'network' | 'venueIndex' | 'schematic'>('venue');

  // Venue Index Search filter
  const [venueSearchQuery, setVenueSearchQuery] = useState<string>('');

  // Dismissible Educational Header Banner
  const [bannerDismissed, setBannerDismissed] = useState<boolean>(false);

  // Reference Gallery Modal
  const [showGallery, setShowGallery] = useState<boolean>(false);
  const [galleryTab, setGalleryTab] = useState<'schematic' | 'masterplan' | 'ab1_mess' | 'lhc_aic' | 'sports'>('schematic');

  // Operation Results strictly from C engine
  const [traversalResult, setTraversalResult] = useState<TraversalResult | null>(null);
  const [routeResult, setRouteResult] = useState<RouteResult | null>(null);
  const [componentsResult, setComponentsResult] = useState<ComponentsResult | null>(null);
  const [networkState, setNetworkState] = useState<NetworkState | null>(null);
  const [failureTable, setFailureTable] = useState<FailureTableResult | null>(null);

  // Theme state: dark | light | high-contrast
  const [theme, setTheme] = useState<'dark' | 'light' | 'high-contrast'>('dark');

  useEffect(() => {
    document.documentElement.setAttribute('data-theme', theme);
  }, [theme]);

  // Hover state for F1 SVG
  const [hoveredVertex, setHoveredVertex] = useState<number | null>(null);
  const [hoveredEdge, setHoveredEdge] = useState<number | null>(null);

  // Refresh active computations strictly from C engine
  const recomputeAll = useCallback(
    (b: RouteGuardBridge, src: number, dst: number, matrixMode: boolean) => {
      const g = b.getGraph();
      setGraph(g);

      const r = b.findRoute(src, dst, matrixMode);
      setRouteResult(r);

      const bfs = b.runBFS(src, dst, matrixMode);
      setTraversalResult(bfs);

      const comp = b.getComponents(matrixMode);
      setComponentsResult(comp);

      const state = b.getNetworkState(src, dst, matrixMode);
      setNetworkState(state);

      const failTbl = b.getFailureTable(src, dst);
      setFailureTable(failTbl);
    },
    []
  );

  // 1. Initialize WASM Bridge with MUJ Campus Demo by default (fixture 1)
  useEffect(() => {
    async function loadWasm() {
      try {
        setLoading(true);
        const b = await getRouteGuardBridge(1);
        setBridge(b);
        setGraphMode('campus');
        setSrcVertex(0);
        setDstVertex(6);
        setSelectedVertexId(0);
        recomputeAll(b, 0, 6, false);
        setLoading(false);
      } catch (err: unknown) {
        console.error('Failed to initialize RouteGuard WASM module:', err);
        setErrorMsg('Failed to instantiate WebAssembly engine. Please check your browser WebAssembly support.');
        setLoading(false);
      }
    }
    loadWasm();
  }, [recomputeAll]);

  // Mode Switch Handler (C-driven fixture selection)
  const handleSwitchMode = (mode: 'campus' | 'f1') => {
    if (!bridge) return;
    const fixtureId = mode === 'campus' ? 1 : 0;
    bridge.initFixture(fixtureId);
    setGraphMode(mode);

    const newSrc = 0;
    const newDst = mode === 'campus' ? 6 : 10;
    setSrcVertex(newSrc);
    setDstVertex(newDst);
    setSelectedVertexId(newSrc);
    recomputeAll(bridge, newSrc, newDst, useMatrix);
  };

  // Walkway Edge Toggle Handler
  const handleToggleEdge = (edgeId: number) => {
    if (!bridge || !graph) return;
    const currentEdge = graph.edges.find((e) => e.id === edgeId);
    if (!currentEdge) return;

    bridge.setEdgeActive(edgeId, !currentEdge.active);
    recomputeAll(bridge, srcVertex, dstVertex, useMatrix);
  };

  // Reset Graph to default authoritative state
  const handleReset = () => {
    if (!bridge) return;
    bridge.reset();
    const defaultDst = graphMode === 'campus' ? 6 : 10;
    setSrcVertex(0);
    setDstVertex(defaultDst);
    setSelectedVertexId(0);
    recomputeAll(bridge, 0, defaultDst, useMatrix);
  };

  // Traversal & Algorithm Handlers
  const handleRunBFS = () => {
    if (!bridge) return;
    const res = bridge.runBFS(srcVertex, dstVertex, useMatrix);
    setTraversalResult(res);
    const r = bridge.findRoute(srcVertex, dstVertex, useMatrix);
    setRouteResult(r);
    setInspectorTab('route');
  };

  const handleRunDFS = () => {
    if (!bridge) return;
    const res = bridge.runDFS(srcVertex, dstVertex, useMatrix);
    setTraversalResult(res);
    setInspectorTab('route');
  };

  const handleFindRoute = () => {
    if (!bridge) return;
    const r = bridge.findRoute(srcVertex, dstVertex, useMatrix);
    setRouteResult(r);
    setInspectorTab('route');
  };

  const handleShowComponents = () => {
    if (!bridge) return;
    const comp = bridge.getComponents(useMatrix);
    setComponentsResult(comp);
    setInspectorTab('network');
  };

  const handleRepresentationChange = (matrix: boolean) => {
    setUseMatrix(matrix);
    if (bridge) {
      recomputeAll(bridge, srcVertex, dstVertex, matrix);
    }
  };

  // Export SVG directly from authoritative C engine
  const handleExportSVG = () => {
    if (!bridge) return;
    const svgStr = bridge.exportSVG(srcVertex, dstVertex, useMatrix);
    const blob = new Blob([svgStr], { type: 'image/svg+xml' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `routeguard_${graphMode}_src${srcVertex}_dst${dstVertex}.svg`;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
  };

  // Route Edge Map for Highlighting
  const routeEdgeMap = useMemo(() => {
    const map = new Map<string, number>();
    if (!routeResult || !routeResult.found || routeResult.path.length < 2) return map;

    for (let i = 0; i < routeResult.path.length - 1; i++) {
      const u = routeResult.path[i];
      const v = routeResult.path[i + 1];
      map.set(`${u}-${v}`, i + 1);
      map.set(`${v}-${u}`, i + 1);
    }
    return map;
  }, [routeResult]);

  // Set of reached vertices
  const reachedSet = useMemo(() => {
    const set = new Set<number>();
    if (traversalResult && traversalResult.order) {
      traversalResult.order.forEach((v) => set.add(v));
    } else if (graph) {
      for (let i = 0; i < graph.n_vertices; i++) set.add(i);
    }
    return set;
  }, [traversalResult, graph]);

  // Selected vertex details
  const selectedVertex = useMemo(() => {
    if (selectedVertexId === null || !graph) return null;
    return graph.vertices.find((v) => v.id === selectedVertexId) ?? null;
  }, [selectedVertexId, graph]);

  const selectedVertexIncidentEdges = useMemo(() => {
    if (selectedVertexId === null || !graph) return [];
    return graph.edges.filter((e) => e.u === selectedVertexId || e.v === selectedVertexId);
  }, [selectedVertexId, graph]);

  // Geo metadata for selected venue (if campus mode)
  const selectedVenueGeo = useMemo(() => {
    if (graphMode !== 'campus' || selectedVertexId === null) return null;
    return CAMPUS_VENUES.find((v) => v.id === selectedVertexId) ?? null;
  }, [graphMode, selectedVertexId]);

  // Filtered venue index for searchable right-tab
  const filteredVenues = useMemo(() => {
    if (!graph) return [];
    const q = venueSearchQuery.trim().toLowerCase();
    return graph.vertices.filter((v) => {
      if (!q) return true;
      const geo = CAMPUS_VENUES.find((cg) => cg.id === v.id);
      return (
        v.label.toLowerCase().includes(q) ||
        (v.full_name && v.full_name.toLowerCase().includes(q)) ||
        (geo && geo.category.toLowerCase().includes(q)) ||
        `node ${v.id}`.includes(q)
      );
    });
  }, [graph, venueSearchQuery]);

  if (loading) {
    return (
      <div className="app-container" style={{ justifyContent: 'center', alignItems: 'center', minHeight: '100vh' }}>
        <div style={{ textAlign: 'center' }}>
          <div style={{ fontSize: '28px', fontWeight: 800, color: 'var(--cyan)', letterSpacing: '-0.5px' }}>
            RouteGuard
          </div>
          <div style={{ color: 'var(--text-secondary)', marginTop: '8px', fontSize: '14px' }}>
            Initializing ISO C11 WebAssembly Engine &amp; MUJ Campus Topology...
          </div>
        </div>
      </div>
    );
  }

  if (errorMsg) {
    return (
      <div className="app-container" style={{ justifyContent: 'center', alignItems: 'center', minHeight: '100vh' }}>
        <div style={{ background: 'rgba(239, 68, 68, 0.15)', border: '1px solid var(--rose)', padding: '24px', borderRadius: '8px', maxWidth: '520px' }}>
          <div style={{ color: 'var(--rose)', fontWeight: 700, fontSize: '18px' }}>Engine Error</div>
          <div style={{ marginTop: '8px', color: '#fca5a5' }}>{errorMsg}</div>
        </div>
      </div>
    );
  }

  const activeEdgeCount = graph?.edges.filter((e) => e.active).length ?? 0;
  const closedEdgeCount = (graph?.n_edges ?? 0) - activeEdgeCount;

  return (
    <div className="app-container">
      {/* 1. Compact Modern Header */}
      <header className="app-header">
        <div className="header-left">
          {/* Left panel collapse toggle */}
          <button
            type="button"
            className={`btn-panel-toggle ${leftPanelOpen ? 'active' : ''}`}
            onClick={() => setLeftPanelOpen(!leftPanelOpen)}
            title={leftPanelOpen ? 'Collapse Left Control Panel' : 'Expand Left Control Panel'}
            aria-label="Toggle Controls Panel"
          >
            ☰
          </button>

          <div className="brand-section">
            <div className="brand-icon">RG</div>
            <div className="brand-text">
              <div className="brand-title">
                RouteGuard
                <span className="brand-badge">WASM / C11</span>
              </div>
              <div className="brand-subtitle">
                {graphMode === 'campus' ? 'MUJ Campus Network Resilience Simulator' : 'F1 Reference Graph Console'}
              </div>
            </div>
          </div>
        </div>

        {/* Center: Mode Selector + Compact Educational Badge */}
        <div className="header-center">
          <div className="mode-switch-group" role="group" aria-label="Graph Environment Mode">
            <button
              type="button"
              className={`mode-btn ${graphMode === 'campus' ? 'active' : ''}`}
              onClick={() => handleSwitchMode('campus')}
              id="mode-campus-btn"
            >
              🏛️ MUJ Campus Demo
            </button>
            <button
              type="button"
              className={`mode-btn ${graphMode === 'f1' ? 'active' : ''}`}
              onClick={() => handleSwitchMode('f1')}
              id="mode-f1-btn"
            >
              📐 F1 Reference Graph
            </button>
          </div>

          {!bannerDismissed && (
            <div className="compact-educational-pill" id="educational-banner">
              <span className="pill-icon">⚠️</span>
              <span className="pill-text">Educational simulation · hop-count routing · not real navigation</span>
              <button
                type="button"
                className="pill-close-btn"
                onClick={() => setBannerDismissed(true)}
                title="Dismiss banner"
                aria-label="Dismiss educational disclaimer"
              >
                ✕
              </button>
            </div>
          )}
        </div>

        {/* Right: Network Status, Theme, Quick Actions, Right Panel Toggle */}
        <div className="header-right">
          <span className={`status-pill ${routeResult?.found ? 'success' : 'error'}`}>
            <span className={`status-indicator-dot ${routeResult?.found ? 'online' : 'offline'}`}></span>
            {routeResult?.found ? `${routeResult.hop_count} Hops` : 'Blocked'}
          </span>

          <div className="theme-toggle-group" role="group" aria-label="Theme selection">
            <button
              type="button"
              className={`theme-btn ${theme === 'dark' ? 'active' : ''}`}
              onClick={() => setTheme('dark')}
              id="theme-dark-btn"
              title="Dark Mode"
            >
              🌙
            </button>
            <button
              type="button"
              className={`theme-btn ${theme === 'light' ? 'active' : ''}`}
              onClick={() => setTheme('light')}
              id="theme-light-btn"
              title="Light Mode"
            >
              ☀️
            </button>
            <button
              type="button"
              className={`theme-btn ${theme === 'high-contrast' ? 'active' : ''}`}
              onClick={() => setTheme('high-contrast')}
              id="theme-hc-btn"
              title="High Contrast"
            >
              👁️
            </button>
          </div>

          <button
            type="button"
            className="btn btn-secondary btn-sm"
            onClick={handleReset}
            id="btn-header-reset"
            title="Reset network to initial default state"
          >
            ↺ Reset
          </button>
          <button
            type="button"
            className="btn btn-primary btn-sm"
            onClick={handleExportSVG}
            id="btn-export-svg"
            title="Export authoritative SVG graphic from C engine"
          >
            ↓ Export SVG
          </button>

          {/* Right panel collapse toggle */}
          <button
            type="button"
            className={`btn-panel-toggle ${rightPanelOpen ? 'active' : ''}`}
            onClick={() => setRightPanelOpen(!rightPanelOpen)}
            title={rightPanelOpen ? 'Collapse Right Inspector' : 'Expand Right Inspector'}
            aria-label="Toggle Inspector Panel"
          >
            📊
          </button>
        </div>
      </header>

      {/* 2. Main Geospatial Content Area */}
      <main className="main-content-layout">
        {/* Panel A: Left Collapsible Control Panel */}
        <aside
          className={`sidebar-panel left-panel ${leftPanelOpen ? 'open' : 'collapsed'}`}
          aria-label="Network Controls"
        >
          <div className="sidebar-inner">
            <div className="sidebar-header">
              <span className="sidebar-title">Network Controls</span>
              <span className="sidebar-tag">{graphMode === 'campus' ? '13 Venues' : '12 Nodes'}</span>
            </div>

            {/* Section 1: Route Planner */}
            <div className="control-section">
              <div className="section-title">
                <span>📍 Route Planner</span>
              </div>

              <div className="control-group">
                <label className="control-label" htmlFor="src-select">
                  <span>Source Venue</span>
                  <span className="badge-src">[SRC]</span>
                </label>
                <select
                  id="src-select"
                  className="form-select"
                  value={srcVertex}
                  onChange={(e) => {
                    const val = Number(e.target.value);
                    setSrcVertex(val);
                    setSelectedVertexId(val);
                    if (bridge) recomputeAll(bridge, val, dstVertex, useMatrix);
                  }}
                >
                  {graph?.vertices.map((v) => (
                    <option key={v.id} value={v.id}>
                      {v.id}: {v.label}
                    </option>
                  ))}
                </select>
              </div>

              <div className="control-group">
                <label className="control-label" htmlFor="dst-select">
                  <span>Destination Venue</span>
                  <span className="badge-dst">[DST]</span>
                </label>
                <select
                  id="dst-select"
                  className="form-select"
                  value={dstVertex}
                  onChange={(e) => {
                    const val = Number(e.target.value);
                    setDstVertex(val);
                    setSelectedVertexId(val);
                    if (bridge) recomputeAll(bridge, srcVertex, val, useMatrix);
                  }}
                >
                  {graph?.vertices.map((v) => (
                    <option key={v.id} value={v.id}>
                      {v.id}: {v.label}
                    </option>
                  ))}
                </select>
              </div>

              <button
                type="button"
                className="btn btn-primary btn-block"
                onClick={handleFindRoute}
                id="btn-find-route"
              >
                🔍 Find Minimum-Hop Route
              </button>
            </div>

            {/* Section 2: Graph Algorithms */}
            <div className="control-section">
              <div className="section-title">
                <span>⚡ Graph Algorithms</span>
              </div>
              <div className="button-stack">
                <button
                  type="button"
                  className="btn btn-secondary btn-sm"
                  onClick={handleRunBFS}
                  id="btn-run-bfs"
                >
                  Run BFS Traversal
                </button>
                <button
                  type="button"
                  className="btn btn-secondary btn-sm"
                  onClick={handleRunDFS}
                  id="btn-run-dfs"
                >
                  Run DFS Traversal
                </button>
                <button
                  type="button"
                  className="btn btn-secondary btn-sm"
                  onClick={handleShowComponents}
                  id="btn-show-components"
                >
                  Analyze Connected Components
                </button>
              </div>
            </div>

            {/* Section 3: Failure Simulation */}
            <div className="control-section">
              <div className="section-title">
                <span>🚧 Failure Simulation</span>
                <span style={{ fontSize: '11px', color: 'var(--amber)' }}>
                  {closedEdgeCount > 0 ? `${closedEdgeCount} Severed` : 'All Active'}
                </span>
              </div>

              {/* Quick toggle dropdown */}
              <div className="control-group">
                <label className="control-label" htmlFor="select-toggle-edge">
                  <span>Toggle Walkway</span>
                </label>
                <select
                  className="form-select"
                  value=""
                  onChange={(e) => {
                    const id = Number(e.target.value);
                    if (!isNaN(id)) handleToggleEdge(id);
                  }}
                  id="select-toggle-edge"
                >
                  <option value="" disabled>
                    Select walkway to toggle...
                  </option>
                  {graph?.edges.map((e) => (
                    <option key={e.id} value={e.id}>
                      e{e.id}: {graph.vertices[e.u]?.label} ↔ {graph.vertices[e.v]?.label} ({e.active ? 'ACTIVE' : 'CLOSED'})
                    </option>
                  ))}
                </select>
              </div>

              {/* Mode-specific quick scenarios */}
              <div className="scenario-stack">
                {graphMode === 'campus' ? (
                  <>
                    <button
                      type="button"
                      className={`btn btn-sm ${graph?.edges[6]?.active ? 'btn-danger' : 'btn-success'}`}
                      onClick={() => handleToggleEdge(6)}
                      id="btn-scenario-campus-e6"
                    >
                      {graph?.edges[6]?.active
                        ? 'Sever e6 (AB1↔AB3 Cut → Reroute via LHC)'
                        : 'Restore e6 (Reconnect AB1↔AB3)'}
                    </button>
                    <button
                      type="button"
                      className={`btn btn-sm ${graph?.edges[12]?.active ? 'btn-danger' : 'btn-success'}`}
                      onClick={() => {
                        handleToggleEdge(12);
                        setDstVertex(9);
                        if (bridge) recomputeAll(bridge, srcVertex, 9, useMatrix);
                      }}
                      id="btn-scenario-campus-e12"
                    >
                      {graph?.edges[12]?.active
                        ? 'Sever e12 (LHC↔Football Cut → Disconnect Sports)'
                        : 'Restore e12 (Reconnect Sports Zone)'}
                    </button>
                  </>
                ) : (
                  <>
                    <button
                      type="button"
                      className={`btn btn-sm ${graph?.edges[9]?.active ? 'btn-danger' : 'btn-success'}`}
                      onClick={() => handleToggleEdge(9)}
                      id="btn-scenario-f1-e9"
                    >
                      {graph?.edges[9]?.active ? 'Sever e9 (Cut Critical Link 6↔7)' : 'Restore e9 (Reconnect 6↔7)'}
                    </button>
                    <button
                      type="button"
                      className={`btn btn-sm ${graph?.edges[5]?.active ? 'btn-danger' : 'btn-success'}`}
                      onClick={() => handleToggleEdge(5)}
                      id="btn-scenario-f1-e5"
                    >
                      {graph?.edges[5]?.active ? 'Sever e5 (Simulate Block A Cut)' : 'Restore e5 (Direct Route)'}
                    </button>
                  </>
                )}
                <button
                  type="button"
                  className="btn btn-secondary btn-sm"
                  onClick={handleReset}
                  id="btn-restore-all"
                >
                  ↺ Restore All Walkways
                </button>
              </div>
            </div>

            {/* Section 4: Graph Representation Switch */}
            <div className="control-section">
              <div className="section-title">
                <span>⚙️ Storage Representation</span>
              </div>
              <div className="seg-switch" role="group" aria-label="Graph Data Structure Representation">
                <button
                  type="button"
                  className={`seg-btn ${!useMatrix ? 'active' : ''}`}
                  onClick={() => handleRepresentationChange(false)}
                  id="btn-rep-list"
                >
                  Adjacency List
                </button>
                <button
                  type="button"
                  className={`seg-btn ${useMatrix ? 'active' : ''}`}
                  onClick={() => handleRepresentationChange(true)}
                  id="btn-rep-matrix"
                >
                  Adjacency Matrix
                </button>
              </div>
              <div className="rep-complexity-caption">
                {useMatrix ? (
                  <span>
                    Matrix: <strong style={{ color: 'var(--amber)' }}>O(V²)</strong> = {((graph?.n_vertices ?? 0) * (graph?.n_vertices ?? 0))} cells.
                    Examines all columns.
                  </span>
                ) : (
                  <span>
                    List: <strong style={{ color: 'var(--emerald)' }}>O(V + E)</strong> = {graph?.n_vertices ?? 0} heads + {((graph?.n_edges ?? 0) * 2)} nodes.
                    Examines incident edges only.
                  </span>
                )}
              </div>
            </div>
          </div>
        </aside>

        {/* Panel B: Center Viewport (Map-Centric Canvas, 65-75% width) */}
        <section className="center-viewport" aria-label="Campus Network Viewport">
          {/* Main Visualizer: Campus Map (MapLibre) or F1 Benchmark Graph (SVG) */}
          <div className="canvas-container">
            {graphMode === 'campus' ? (
              <CampusMap
                graph={graph}
                srcVertex={srcVertex}
                dstVertex={dstVertex}
                selectedVertexId={selectedVertexId}
                routeResult={routeResult}
                traversalResult={traversalResult}
                theme={theme}
                onSelectVertex={(vId) => {
                  setSelectedVertexId(vId);
                  setInspectorTab('venue');
                }}
                onSetSource={(vId) => {
                  setSrcVertex(vId);
                  if (bridge) recomputeAll(bridge, vId, dstVertex, useMatrix);
                }}
                onSetDestination={(vId) => {
                  setDstVertex(vId);
                  if (bridge) recomputeAll(bridge, srcVertex, vId, useMatrix);
                }}
                onToggleEdge={handleToggleEdge}
                onOpenSchematic={() => {
                  setGalleryTab('schematic');
                  setShowGallery(true);
                }}
              />
            ) : (
              /* F1 Reference Fixture SVG Canvas */
              <div className="f1-canvas-wrapper">
                <div className="f1-toolbar">
                  <span className="status-pill info">📐 F1 Benchmark Fixture (12 Vertices, 13 Edges)</span>
                  <span style={{ fontSize: '12px', color: 'var(--text-muted)' }}>
                    Standard deterministic test benchmark for CSC2101 validation.
                  </span>
                </div>
                <div className="svg-canvas-wrapper">
                  <svg
                    className="svg-canvas"
                    viewBox="0 0 1180 430"
                    preserveAspectRatio="xMidYMid meet"
                    xmlns="http://www.w3.org/2000/svg"
                    role="region"
                    aria-label="F1 Graph Canvas"
                  >
                    <defs>
                      <filter id="f1-glow-cyan" x="-20%" y="-20%" width="140%" height="140%">
                        <feGaussianBlur stdDeviation="3" result="blur" />
                        <feComposite in="SourceGraphic" in2="blur" operator="over" />
                      </filter>
                      <filter id="f1-glow-red" x="-20%" y="-20%" width="140%" height="140%">
                        <feGaussianBlur stdDeviation="4" result="blur" />
                        <feComposite in="SourceGraphic" in2="blur" operator="over" />
                      </filter>
                    </defs>

                    {/* Edge Connections */}
                    {graph?.edges.map((e) => {
                      const uNode = graph.vertices[e.u];
                      const vNode = graph.vertices[e.v];
                      if (!uNode || !vNode) return null;

                      const edgeKey = `${e.u}-${e.v}`;
                      const routeHop = routeEdgeMap.get(edgeKey);
                      const isHovered = hoveredEdge === e.id;
                      const midX = (uNode.x + vNode.x) / 2;
                      const midY = (uNode.y + vNode.y) / 2;

                      return (
                        <g
                          key={`f1-edge-${e.id}`}
                          role="button"
                          tabIndex={0}
                          aria-label={`Edge e${e.id} between ${uNode.label} and ${vNode.label}`}
                          onClick={() => handleToggleEdge(e.id)}
                          onKeyDown={(evt) => {
                            if (evt.key === 'Enter' || evt.key === ' ') {
                              evt.preventDefault();
                              handleToggleEdge(e.id);
                            }
                          }}
                          onMouseEnter={() => setHoveredEdge(e.id)}
                          onMouseLeave={() => setHoveredEdge(null)}
                          style={{ cursor: 'pointer' }}
                        >
                          <line
                            x1={uNode.x}
                            y1={uNode.y}
                            x2={vNode.x}
                            y2={vNode.y}
                            stroke="transparent"
                            strokeWidth={18}
                          />

                          {!e.active ? (
                            <>
                              <line
                                x1={uNode.x}
                                y1={uNode.y}
                                x2={vNode.x}
                                y2={vNode.y}
                                stroke="#ef4444"
                                strokeWidth={2.5}
                                strokeDasharray="6,4"
                                filter={isHovered ? 'url(#f1-glow-red)' : undefined}
                              />
                              <rect
                                x={midX - 26}
                                y={midY - 9}
                                width={52}
                                height={18}
                                rx={4}
                                fill="#ef4444"
                                stroke="#991b1b"
                              />
                              <text
                                x={midX}
                                y={midY + 4}
                                fill="#ffffff"
                                fontSize={10}
                                fontWeight="bold"
                                textAnchor="middle"
                                fontFamily="var(--font-mono)"
                              >
                                CLOSED
                              </text>
                            </>
                          ) : routeHop !== undefined ? (
                            <>
                              <line
                                x1={uNode.x}
                                y1={uNode.y}
                                x2={vNode.x}
                                y2={vNode.y}
                                stroke="#38bdf8"
                                strokeWidth={isHovered ? 5 : 4}
                                filter="url(#f1-glow-cyan)"
                              />
                              <circle
                                cx={midX}
                                cy={midY}
                                r={10}
                                fill="#0284c7"
                                stroke="#38bdf8"
                                strokeWidth={1.5}
                              />
                              <text
                                x={midX}
                                y={midY + 3.5}
                                fill="#ffffff"
                                fontSize={10}
                                fontWeight="bold"
                                textAnchor="middle"
                                fontFamily="var(--font-mono)"
                              >
                                {routeHop}
                              </text>
                            </>
                          ) : (
                            <line
                              x1={uNode.x}
                              y1={uNode.y}
                              x2={vNode.x}
                              y2={vNode.y}
                              stroke={isHovered ? 'var(--edge-hover)' : 'var(--edge-stroke)'}
                              strokeWidth={isHovered ? 3 : 2}
                            />
                          )}
                        </g>
                      );
                    })}

                    {/* Vertices */}
                    {graph?.vertices.map((v) => {
                      const isSrc = v.id === srcVertex;
                      const isDst = v.id === dstVertex;
                      const isSelected = v.id === selectedVertexId;
                      const isReached = reachedSet.has(v.id);
                      const isHovered = hoveredVertex === v.id;

                      let fillColor = 'var(--node-fill)';
                      let strokeColor = 'var(--node-stroke)';
                      let strokeWidth = 2;

                      if (isSrc) {
                        fillColor = '#065f46';
                        strokeColor = '#10b981';
                        strokeWidth = 3;
                      } else if (isDst) {
                        fillColor = '#075985';
                        strokeColor = '#38bdf8';
                        strokeWidth = 3;
                      } else if (!isReached) {
                        fillColor = 'var(--bg-app)';
                        strokeColor = 'var(--rose)';
                      }

                      return (
                        <g
                          key={`f1-node-${v.id}`}
                          role="button"
                          tabIndex={0}
                          aria-label={`Node ${v.id}: ${v.label}`}
                          onClick={() => {
                            setSelectedVertexId(v.id);
                            setInspectorTab('venue');
                            if (v.id !== srcVertex) {
                              setDstVertex(v.id);
                              if (bridge) recomputeAll(bridge, srcVertex, v.id, useMatrix);
                            }
                          }}
                          onContextMenu={(e) => {
                            e.preventDefault();
                            setSelectedVertexId(v.id);
                            setSrcVertex(v.id);
                            if (bridge) recomputeAll(bridge, v.id, dstVertex, useMatrix);
                          }}
                          onMouseEnter={() => setHoveredVertex(v.id)}
                          onMouseLeave={() => setHoveredVertex(null)}
                          style={{ cursor: 'pointer' }}
                        >
                          {(isSrc || isDst || isSelected || isHovered) && (
                            <circle
                              cx={v.x}
                              cy={v.y}
                              r={24}
                              fill="none"
                              stroke={isSrc ? '#10b981' : isDst ? '#38bdf8' : isSelected ? 'var(--cyan)' : 'var(--border-medium)'}
                              strokeWidth={isSelected ? 2 : 1.5}
                              strokeDasharray={isHovered ? '3,3' : undefined}
                            />
                          )}

                          <circle
                            cx={v.x}
                            cy={v.y}
                            r={18}
                            fill={fillColor}
                            stroke={strokeColor}
                            strokeWidth={strokeWidth}
                            strokeDasharray={!isReached ? '4,3' : undefined}
                          />

                          <text
                            x={v.x}
                            y={v.y + 4.5}
                            fill="var(--node-text)"
                            fontSize={12}
                            fontWeight="bold"
                            textAnchor="middle"
                            fontFamily="var(--font-mono)"
                          >
                            {v.id}
                          </text>

                          <g transform={`translate(${v.x}, ${v.y + 24})`}>
                            <rect
                              x={-45}
                              y={0}
                              width={90}
                              height={18}
                              rx={4}
                              fill="var(--bg-surface)"
                              stroke="var(--border-medium)"
                              opacity={0.95}
                            />
                            <text
                              x={0}
                              y={12.5}
                              fill="var(--text-main)"
                              fontSize={10.5}
                              fontWeight="600"
                              textAnchor="middle"
                            >
                              {v.label}
                            </text>
                          </g>

                          {isSrc && (
                            <g transform={`translate(${v.x}, ${v.y - 25})`}>
                              <rect x={-16} y={0} width={32} height={14} rx={3} fill="#10b981" />
                              <text x={0} y={10.5} fill="#040914" fontSize={9} fontWeight="800" textAnchor="middle">
                                SRC
                              </text>
                            </g>
                          )}
                          {isDst && (
                            <g transform={`translate(${v.x}, ${v.y - 25})`}>
                              <rect x={-16} y={0} width={32} height={14} rx={3} fill="#38bdf8" />
                              <text x={0} y={10.5} fill="#040914" fontSize={9} fontWeight="800" textAnchor="middle">
                                DST
                              </text>
                            </g>
                          )}
                        </g>
                      );
                    })}
                  </svg>
                </div>
              </div>
            )}

            {/* Floating Route Summary Strip */}
            <div className="route-summary-strip" id="route-summary-strip">
              <div className="strip-item">
                <span className="strip-label">Route</span>
                <span className="strip-value highlight">
                  {routeResult?.found ? `${routeResult.hop_count} Hops` : 'Severed / Blocked'}
                </span>
              </div>
              <div className="strip-divider"></div>
              <div className="strip-item strip-path-box">
                <span className="strip-label">Walkway Path</span>
                <div className="strip-path-scroll">
                  {routeResult?.found && routeResult.path.length > 0 ? (
                    routeResult.path.map((vId, idx) => (
                      <React.Fragment key={`strip-v-${vId}-${idx}`}>
                        <span
                          className={`strip-node-chip ${vId === srcVertex ? 'src' : vId === dstVertex ? 'dst' : ''}`}
                          onClick={() => {
                            setSelectedVertexId(vId);
                            setInspectorTab('venue');
                          }}
                        >
                          {vId}: {graph?.vertices[vId]?.label}
                        </span>
                        {idx < routeResult.path.length - 1 && <span className="strip-arrow">→</span>}
                      </React.Fragment>
                    ))
                  ) : (
                    <span style={{ color: 'var(--rose)', fontSize: '12px' }}>
                      No active path connects Vertex {srcVertex} to Vertex {dstVertex}
                    </span>
                  )}
                </div>
              </div>
              <div className="strip-divider"></div>
              <div className="strip-item">
                <span className="strip-label">Representation</span>
                <span className="strip-value">
                  {useMatrix ? 'Matrix O(V²)' : 'List O(V+E)'}
                </span>
              </div>
              <div className="strip-divider"></div>
              <div className="strip-item">
                <span className="strip-label">Components</span>
                <span className="strip-value">
                  {networkState?.component_count ?? 1} {networkState?.component_count === 1 ? '(Connected)' : '(Severed)'}
                </span>
              </div>
            </div>

            {/* Corner Map Legend */}
            <div className="map-legend-corner" aria-label="Map Legend">
              <div className="legend-row">
                <span className="legend-dot active-edge"></span>
                <span>Active Walkway</span>
              </div>
              <div className="legend-row">
                <span className="legend-dot closed-edge"></span>
                <span>Closed / Blocked</span>
              </div>
              <div className="legend-row">
                <span className="legend-dot route-edge"></span>
                <span>Optimal Route Hop</span>
              </div>
              <div className="legend-row">
                <span className="legend-dot src-node"></span>
                <span>Source</span>
              </div>
              <div className="legend-row">
                <span className="legend-dot dst-node"></span>
                <span>Destination</span>
              </div>
            </div>
          </div>
        </section>

        {/* Panel C: Right Collapsible Inspector Panel (~320px) */}
        <aside
          className={`sidebar-panel right-panel ${rightPanelOpen ? 'open' : 'collapsed'}`}
          aria-label="Inspector and Analysis"
        >
          <div className="sidebar-inner">
            {/* Inspector Navigation Tabs */}
            <div className="inspector-tabs" role="tablist">
              <button
                type="button"
                className={`inspector-tab-btn ${inspectorTab === 'venue' ? 'active' : ''}`}
                onClick={() => setInspectorTab('venue')}
                id="tab-venue"
                role="tab"
                aria-selected={inspectorTab === 'venue'}
              >
                🏢 Venue
              </button>
              <button
                type="button"
                className={`inspector-tab-btn ${inspectorTab === 'route' ? 'active' : ''}`}
                onClick={() => setInspectorTab('route')}
                id="tab-route"
                role="tab"
                aria-selected={inspectorTab === 'route'}
              >
                🧭 Route
              </button>
              <button
                type="button"
                className={`inspector-tab-btn ${inspectorTab === 'network' ? 'active' : ''}`}
                onClick={() => setInspectorTab('network')}
                id="tab-network"
                role="tab"
                aria-selected={inspectorTab === 'network'}
              >
                📊 Network
              </button>
              <button
                type="button"
                className={`inspector-tab-btn ${inspectorTab === 'venueIndex' ? 'active' : ''}`}
                onClick={() => setInspectorTab('venueIndex')}
                id="tab-venue-index"
                role="tab"
                aria-selected={inspectorTab === 'venueIndex'}
              >
                📑 Index
              </button>
              <button
                type="button"
                className={`inspector-tab-btn ${inspectorTab === 'schematic' ? 'active' : ''}`}
                onClick={() => setInspectorTab('schematic')}
                id="tab-schematic"
                role="tab"
                aria-selected={inspectorTab === 'schematic'}
              >
                🗺️ Reference
              </button>
            </div>

            <div className="inspector-content">
              {/* Tab 1: Selected Venue Detail */}
              {inspectorTab === 'venue' && (
                <div className="tab-pane">
                  {selectedVertex ? (
                    <div className="venue-inspector-card">
                      <div className="venue-header-box">
                        <div className="venue-id-badge">Node #{selectedVertex.id}</div>
                        <div className="venue-title-text">{selectedVertex.label}</div>
                        {selectedVertex.id === srcVertex && <span className="status-pill success">SOURCE</span>}
                        {selectedVertex.id === dstVertex && <span className="status-pill info">DESTINATION</span>}
                      </div>

                      <div className="venue-desc-box">
                        {selectedVertex.full_name || selectedVertex.label}
                      </div>

                      {selectedVenueGeo && (
                        <div className="geo-metadata-box">
                          <div className="geo-row">
                            <span className="geo-key">Category:</span>
                            <span className="geo-val uppercase">{selectedVenueGeo.category}</span>
                          </div>
                          <div className="geo-row">
                            <span className="geo-key">Coordinates:</span>
                            <span className="geo-val font-mono">
                              {selectedVenueGeo.coordinates[1].toFixed(5)}°N, {selectedVenueGeo.coordinates[0].toFixed(5)}°E
                            </span>
                          </div>
                          <div className="geo-row">
                            <span className="geo-key">Confidence:</span>
                            <span className={`status-pill ${selectedVenueGeo.confidence === 'verified_poi' ? 'success' : 'warning'}`}>
                              {selectedVenueGeo.confidence === 'verified_poi' ? 'Verified Satellite POI' : 'Approximate Alignment'}
                            </span>
                          </div>
                          {selectedVenueGeo.notes && (
                            <div className="geo-notes">
                              {selectedVenueGeo.notes}
                            </div>
                          )}
                        </div>
                      )}

                      <div className="venue-stats-grid">
                        <div className="stat-card">
                          <span className="stat-label">Degree</span>
                          <span className="stat-value">{selectedVertexIncidentEdges.length}</span>
                        </div>
                        <div className="stat-card">
                          <span className="stat-label">Walkways</span>
                          <span className="stat-value">
                            {selectedVertexIncidentEdges.filter((e) => e.active).length} / {selectedVertexIncidentEdges.length} Active
                          </span>
                        </div>
                      </div>

                      {/* Incident Edges List */}
                      <div className="incident-edges-section">
                        <div className="sub-heading">Connected Walkways</div>
                        <div className="edge-list-chips">
                          {selectedVertexIncidentEdges.map((e) => {
                            const otherId = e.u === selectedVertex.id ? e.v : e.u;
                            const otherLabel = graph?.vertices[otherId]?.label ?? otherId;
                            return (
                              <button
                                key={`incident-${e.id}`}
                                type="button"
                                className={`edge-chip-btn ${e.active ? 'active' : 'severed'}`}
                                onClick={() => handleToggleEdge(e.id)}
                                title={`Click to ${e.active ? 'Close' : 'Re-open'} Walkway e${e.id}`}
                              >
                                <span>e{e.id} ↔ {otherLabel}</span>
                                <span className="chip-status">{e.active ? 'ACTIVE' : 'CLOSED'}</span>
                              </button>
                            );
                          })}
                        </div>
                      </div>

                      {/* Quick SRC / DST Buttons */}
                      <div className="venue-action-buttons">
                        <button
                          type="button"
                          className="btn btn-secondary btn-sm"
                          onClick={() => {
                            setSrcVertex(selectedVertex.id);
                            if (bridge) recomputeAll(bridge, selectedVertex.id, dstVertex, useMatrix);
                          }}
                          id="btn-set-src"
                        >
                          🟢 Set as Source
                        </button>
                        <button
                          type="button"
                          className="btn btn-secondary btn-sm"
                          onClick={() => {
                            setDstVertex(selectedVertex.id);
                            if (bridge) recomputeAll(bridge, srcVertex, selectedVertex.id, useMatrix);
                          }}
                          id="btn-set-dst"
                        >
                          🔵 Set as Destination
                        </button>
                      </div>
                    </div>
                  ) : (
                    <div className="empty-state-box">
                      <span>Click any venue on the map to inspect its details and walkways.</span>
                    </div>
                  )}
                </div>
              )}

              {/* Tab 2: Route & Traversal Sequence */}
              {inspectorTab === 'route' && (
                <div className="tab-pane">
                  {/* Route Summary */}
                  <div className={`route-card ${routeResult?.found ? 'found' : 'severed'}`}>
                    <div className="route-card-header">
                      <span className="route-card-title">
                        {routeResult?.found ? 'Minimum-Hop Walkway Path' : 'Destination Unreachable'}
                      </span>
                      <span className={`status-pill ${routeResult?.found ? 'success' : 'error'}`}>
                        {routeResult?.found ? `${routeResult.hop_count} Hops` : 'No Route'}
                      </span>
                    </div>

                    {routeResult?.found ? (
                      <div className="route-card-body">
                        <div className="route-explanation-text">
                          Optimal path computed by ISO C11 Breadth-First Search (BFS level-order predecessor tree):
                        </div>
                        <div className="route-hop-chips">
                          {routeResult.path.map((vId, idx) => (
                            <div key={`route-hop-card-${vId}-${idx}`} className="hop-item">
                              <div
                                className={`hop-chip ${vId === srcVertex ? 'src' : vId === dstVertex ? 'dst' : ''}`}
                                onClick={() => {
                                  setSelectedVertexId(vId);
                                  setInspectorTab('venue');
                                }}
                              >
                                <span className="hop-index">Hop {idx}</span>
                                <span className="hop-label">{vId}: {graph?.vertices[vId]?.label}</span>
                              </div>
                              {idx < routeResult.path.length - 1 && <span className="hop-connector">↓</span>}
                            </div>
                          ))}
                        </div>
                      </div>
                    ) : (
                      <div className="severed-notice">
                        <p>
                          A critical walkway connection has been severed. No active pathway connects
                          <strong> {srcVertex} ({graph?.vertices[srcVertex]?.label})</strong> to
                          <strong> {dstVertex} ({graph?.vertices[dstVertex]?.label})</strong>.
                        </p>
                        <p style={{ marginTop: '8px', fontSize: '11px', color: 'var(--text-muted)' }}>
                          Click any closed walkway on the map or in the Failure Simulation panel to re-open and restore connectivity.
                        </p>
                      </div>
                    )}
                  </div>

                  {/* Traversal Order Breakdown */}
                  <div className="traversal-order-card">
                    <div className="traversal-header">
                      <span className="traversal-title">
                        {traversalResult?.algorithm ?? 'BFS'} Traversal Order
                      </span>
                      <span className="traversal-count">
                        {traversalResult?.order_len ?? 0} Venues Visited
                      </span>
                    </div>
                    <div className="traversal-sequence">
                      {traversalResult?.order?.map((vId, idx) => (
                        <span
                          key={`trav-order-${vId}-${idx}`}
                          className="trav-chip"
                          onClick={() => {
                            setSelectedVertexId(vId);
                            setInspectorTab('venue');
                          }}
                        >
                          {vId}: {graph?.vertices[vId]?.label}
                          {idx < (traversalResult.order.length - 1) && <span className="trav-comma">,</span>}
                        </span>
                      ))}
                    </div>
                  </div>
                </div>
              )}

              {/* Tab 3: C Operation Counters & Failure Simulation */}
              {inspectorTab === 'network' && (
                <div className="tab-pane">
                  <div className="counters-grid">
                    <div className="counter-box">
                      <span className="counter-label">Neighbor Slots Examined</span>
                      <span className="counter-val highlight" style={{ color: useMatrix ? 'var(--amber)' : 'var(--emerald)' }}>
                        {traversalResult?.metrics.neighbor_slots_examined ?? 0}
                      </span>
                      <span className="counter-sub">
                        {useMatrix
                          ? `Matrix V² = ${(graph?.n_vertices ?? 0) * (graph?.n_vertices ?? 0)} potential cells`
                          : `List 2E = ${(graph?.n_edges ?? 0) * 2} stored nodes`}
                      </span>
                    </div>

                    <div className="counter-box">
                      <span className="counter-label">Nodes Processed</span>
                      <span className="counter-val">{traversalResult?.metrics.vertices_processed ?? 0}</span>
                      <span className="counter-sub">Dequeued / Popped</span>
                    </div>

                    <div className="counter-box">
                      <span className="counter-label">Inactive Skips</span>
                      <span className="counter-val">{traversalResult?.metrics.edges_skipped_inactive ?? 0}</span>
                      <span className="counter-sub">Closed walkways bypassed</span>
                    </div>

                    <div className="counter-box">
                      <span className="counter-label">Queue Ops (BFS)</span>
                      <span className="counter-val">
                        {traversalResult?.metrics.queue_enqueues ?? 0} / {traversalResult?.metrics.queue_dequeues ?? 0}
                      </span>
                      <span className="counter-sub">Enqueues / Dequeues</span>
                    </div>

                    <div className="counter-box">
                      <span className="counter-label">Stack Ops (DFS)</span>
                      <span className="counter-val">
                        {traversalResult?.metrics.stack_pushes ?? 0} / {traversalResult?.metrics.stack_pops ?? 0}
                      </span>
                      <span className="counter-sub">Pushes / Pops</span>
                    </div>

                    <div className="counter-box">
                      <span className="counter-label">Stale Pops (DFS)</span>
                      <span className="counter-val" style={{ color: 'var(--amber)' }}>
                        {traversalResult?.metrics.stale_pops ?? 0}
                      </span>
                      <span className="counter-sub">Re-entrant pops skipped</span>
                    </div>
                  </div>

                  {/* Components Breakdown */}
                  <div className="components-status-box">
                    <div className="sub-heading">Partition &amp; Reachability</div>
                    <div className="comp-stats-row">
                      <span>Connected Components: <strong>{networkState?.component_count ?? 1}</strong></span>
                      <span>Reachable from SRC: <strong>{networkState?.reachable_src_count ?? 0} / {graph?.n_vertices ?? 0}</strong></span>
                    </div>
                    {componentsResult && componentsResult.component_count > 1 && (
                      <div className="comp-chips-wrap">
                        {componentsResult.components.map((c) => (
                          <span key={`comp-${c.vertex}`} className="comp-chip">
                            Node {c.vertex}: Comp {c.component_id}
                          </span>
                        ))}
                      </div>
                    )}
                  </div>

                  {/* Single-Edge Failure Simulation Table */}
                  <div className="failure-table-section">
                    <div className="sub-heading">Single-Edge Failure Simulation</div>
                    <p className="table-caption-note">
                      Systematically simulates temporary single-edge closures via repeated C BFS traversals (O(E · (V+E))).
                      Demonstrates operational partition impact without low-link bridge detection algorithms.
                    </p>
                    <div className="table-scroll-wrapper">
                      <table className="failure-data-table">
                        <thead>
                          <tr>
                            <th>Edge</th>
                            <th>Walkway</th>
                            <th>Status</th>
                            <th>Reachable?</th>
                            <th>Hops</th>
                            <th>Comps</th>
                          </tr>
                        </thead>
                        <tbody>
                          {failureTable?.rows.map((row) => {
                            const uLabel = graph?.vertices[row.u]?.label ?? row.u;
                            const vLabel = graph?.vertices[row.v]?.label ?? row.v;
                            const isCritical = !row.reachable;
                            const isClosed = graph?.edges[row.edge_id]?.active === false;

                            return (
                              <tr
                                key={`row-fail-${row.edge_id}`}
                                onClick={() => handleToggleEdge(row.edge_id)}
                                className={isCritical ? 'critical-row' : ''}
                                title="Click to toggle this walkway"
                              >
                                <td className="font-mono bold">e{row.edge_id}</td>
                                <td>{uLabel} ↔ {vLabel}</td>
                                <td>
                                  <span className={`status-pill ${isClosed ? 'error' : 'success'}`}>
                                    {isClosed ? 'CLOSED' : 'ACTIVE'}
                                  </span>
                                </td>
                                <td>
                                  <span style={{ color: row.reachable ? 'var(--emerald)' : 'var(--rose)', fontWeight: 600 }}>
                                    {row.reachable ? 'Yes' : 'Severed'}
                                  </span>
                                </td>
                                <td>{row.hop_count === -1 ? '—' : row.hop_count}</td>
                                <td>{row.component_count}</td>
                              </tr>
                            );
                          })}
                        </tbody>
                      </table>
                    </div>
                  </div>
                </div>
              )}

              {/* Tab 4: Searchable Venue Index */}
              {inspectorTab === 'venueIndex' && (
                <div className="tab-pane">
                  <div className="search-bar-wrap">
                    <input
                      type="text"
                      className="form-input search-input"
                      placeholder="Search venue name, ID or category..."
                      value={venueSearchQuery}
                      onChange={(e) => setVenueSearchQuery(e.target.value)}
                      id="venue-search-input"
                    />
                    {venueSearchQuery && (
                      <button
                        type="button"
                        className="search-clear-btn"
                        onClick={() => setVenueSearchQuery('')}
                        aria-label="Clear search"
                      >
                        ✕
                      </button>
                    )}
                  </div>

                  <div className="venue-index-list">
                    {filteredVenues.map((v) => {
                      const geo = CAMPUS_VENUES.find((cg) => cg.id === v.id);
                      const isSrc = v.id === srcVertex;
                      const isDst = v.id === dstVertex;
                      const isSelected = v.id === selectedVertexId;

                      return (
                        <div
                          key={`index-venue-${v.id}`}
                          className={`venue-index-item ${isSelected ? 'selected' : ''}`}
                          onClick={() => {
                            setSelectedVertexId(v.id);
                            setInspectorTab('venue');
                          }}
                        >
                          <div className="item-main">
                            <span className="item-id">#{v.id}</span>
                            <span className="item-label">{v.label}</span>
                            {isSrc && <span className="status-pill success mini">SRC</span>}
                            {isDst && <span className="status-pill info mini">DST</span>}
                          </div>
                          <div className="item-sub">
                            {geo ? (
                              <>
                                <span className="category-tag">{geo.category}</span>
                                <span className={`confidence-tag ${geo.confidence === 'verified_poi' ? 'verified' : 'approx'}`}>
                                  {geo.confidence === 'verified_poi' ? 'Verified' : 'Approx'}
                                </span>
                              </>
                            ) : (
                              <span className="category-tag">F1 Node</span>
                            )}
                          </div>
                        </div>
                      );
                    })}
                  </div>
                </div>
              )}

              {/* Tab 5: Schematic & Architectural References */}
              {inspectorTab === 'schematic' && (
                <div className="tab-pane">
                  <div className="schematic-pane-card">
                    <div className="sub-heading">MUJ Campus Wayfinding Schematic</div>
                    <p style={{ fontSize: '11px', color: 'var(--text-muted)', marginBottom: '10px' }}>
                      Illustrated cream wayfinding schematic (not to scale). Preserved as supplementary reference.
                    </p>
                    <div
                      className="schematic-preview-box"
                      onClick={() => {
                        setGalleryTab('schematic');
                        setShowGallery(true);
                      }}
                      title="Click to expand schematic reference"
                    >
                      <img
                        src="/maps/muj-campus-wayfinding-schematic.png"
                        alt="MUJ Campus Wayfinding Schematic"
                        className="schematic-thumb"
                      />
                      <div className="preview-overlay">🔍 Click to Expand</div>
                    </div>

                    <div className="reference-buttons-stack">
                      <button
                        type="button"
                        className="btn btn-secondary btn-sm btn-block"
                        onClick={() => {
                          setGalleryTab('ab1_mess');
                          setShowGallery(true);
                        }}
                      >
                        🛰️ AB1 &amp; Old Mess Satellite View
                      </button>
                      <button
                        type="button"
                        className="btn btn-secondary btn-sm btn-block"
                        onClick={() => {
                          setGalleryTab('lhc_aic');
                          setShowGallery(true);
                        }}
                      >
                        🛰️ LHC &amp; AIC-MUJ Satellite View
                      </button>
                      <button
                        type="button"
                        className="btn btn-secondary btn-sm btn-block"
                        onClick={() => {
                          setGalleryTab('sports');
                          setShowGallery(true);
                        }}
                      >
                        🛰️ Sports Grounds Satellite View
                      </button>
                    </div>
                  </div>
                </div>
              )}
            </div>
          </div>
        </aside>
      </main>

      {/* 3. High-Resolution Gallery & Architectural Reference Modal */}
      {showGallery && (
        <div className="gallery-modal-backdrop" onClick={() => setShowGallery(false)}>
          <div className="gallery-modal-card" onClick={(e) => e.stopPropagation()}>
            <div className="gallery-modal-header">
              <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
                <span style={{ fontSize: '20px' }}>🏛️</span>
                <h3 style={{ margin: 0, fontSize: '16px', fontWeight: 700 }}>
                  MUJ Campus Context &amp; Architectural Reference Imagery
                </h3>
              </div>
              <button
                type="button"
                className="btn btn-secondary btn-sm"
                onClick={() => setShowGallery(false)}
                id="btn-close-gallery"
                aria-label="Close Reference Modal"
              >
                ✕ Close
              </button>
            </div>

            <div className="gallery-tabs">
              <button
                type="button"
                className={`gallery-tab-btn ${galleryTab === 'schematic' ? 'active' : ''}`}
                onClick={() => setGalleryTab('schematic')}
              >
                Wayfinding Schematic (Hafeez Contractor)
              </button>
              <button
                type="button"
                className={`gallery-tab-btn ${galleryTab === 'masterplan' ? 'active' : ''}`}
                onClick={() => setGalleryTab('masterplan')}
              >
                Hafeez Site Master Plan
              </button>
              <button
                type="button"
                className={`gallery-tab-btn ${galleryTab === 'ab1_mess' ? 'active' : ''}`}
                onClick={() => setGalleryTab('ab1_mess')}
              >
                AB1 &amp; Old Mess Satellite
              </button>
              <button
                type="button"
                className={`gallery-tab-btn ${galleryTab === 'lhc_aic' ? 'active' : ''}`}
                onClick={() => setGalleryTab('lhc_aic')}
              >
                LHC &amp; AIC-MUJ Satellite
              </button>
              <button
                type="button"
                className={`gallery-tab-btn ${galleryTab === 'sports' ? 'active' : ''}`}
                onClick={() => setGalleryTab('sports')}
              >
                Football &amp; Sports Satellite
              </button>
            </div>

            <div className="gallery-image-viewport">
              {galleryTab === 'schematic' && (
                <div className="gallery-item">
                  <img
                    src="/maps/muj-campus-wayfinding-schematic.png"
                    alt="MUJ Campus Wayfinding Schematic"
                    className="gallery-img"
                  />
                  <p className="gallery-caption">
                    <strong>Official MUJ Campus Wayfinding Schematic</strong> — Cream-toned orientation drawing depicting Academic Blocks 1–3, Dome Building, Old Mess, LHC, Workshops, and Sports complex. Illustrative schematic layout, not to scale.
                  </p>
                </div>
              )}
              {galleryTab === 'masterplan' && (
                <div className="gallery-item">
                  <img
                    src="/maps/muj-campus-masterplan.png"
                    alt="MUJ Campus Master Site Plan by Architect Hafeez Contractor"
                    className="gallery-img"
                  />
                  <p className="gallery-caption">
                    <strong>Official Site Master Plan (Hafeez Contractor, AHC)</strong> — Division of Academic Zone and Residential Zone with central HT line axis. Used as supplementary architectural context.
                  </p>
                </div>
              )}
              {galleryTab === 'ab1_mess' && (
                <div className="gallery-item">
                  <img
                    src="/maps/muj-academic-block-old-mess-satellite.png"
                    alt="Academic Block 1, Old Mess, Knowledge Centre Satellite"
                    className="gallery-img"
                  />
                  <p className="gallery-caption">
                    <strong>Academic Block 1 &amp; Old Mess Satellite View</strong> — Academic Block 1 (courtyard structure), Old Mess / Food Court, and Knowledge Centre. Map imagery &copy; Google Maps.
                  </p>
                </div>
              )}
              {galleryTab === 'lhc_aic' && (
                <div className="gallery-item">
                  <img
                    src="/maps/muj-lecture-hall-aic-satellite.png"
                    alt="Lecture Hall Complex, AIC-MUJ Satellite"
                    className="gallery-img"
                  />
                  <p className="gallery-caption">
                    <strong>Lecture Hall Complex &amp; AIC-MUJ Satellite View</strong> — Tiered lecture halls (LHC), AIC-MUJ innovation hub, and connecting walkways. Map imagery &copy; Google Maps.
                  </p>
                </div>
              )}
              {galleryTab === 'sports' && (
                <div className="gallery-item">
                  <img
                    src="/maps/muj-football-ground-satellite.png"
                    alt="MUJ Football Ground Satellite"
                    className="gallery-img"
                  />
                  <p className="gallery-caption">
                    <strong>MUJ Football Ground &amp; Sports Zone Satellite View</strong> — Football turf, cricket ground perimeter, and nearby Lecture Hall Complex. Map imagery &copy; Google Maps.
                  </p>
                </div>
              )}
            </div>
          </div>
        </div>
      )}
    </div>
  );
};

export default App;
