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

export const App: React.FC = () => {
  const [bridge, setBridge] = useState<RouteGuardBridge | null>(null);
  const [graph, setGraph] = useState<GraphData | null>(null);
  const [loading, setLoading] = useState<boolean>(true);
  const [errorMsg, setErrorMsg] = useState<string | null>(null);

  // Controls
  const [srcVertex, setSrcVertex] = useState<number>(0);
  const [dstVertex, setDstVertex] = useState<number>(10);
  const [useMatrix, setUseMatrix] = useState<boolean>(false);
  const [activeTab, setActiveTab] = useState<'routing' | 'metrics' | 'failureTable'>('routing');

  // Operation Results from C engine
  const [traversalResult, setTraversalResult] = useState<TraversalResult | null>(null);
  const [routeResult, setRouteResult] = useState<RouteResult | null>(null);
  const [componentsResult, setComponentsResult] = useState<ComponentsResult | null>(null);
  const [networkState, setNetworkState] = useState<NetworkState | null>(null);
  const [failureTable, setFailureTable] = useState<FailureTableResult | null>(null);

  // Tooltip & Hover state
  const [hoveredVertex, setHoveredVertex] = useState<number | null>(null);
  const [hoveredEdge, setHoveredEdge] = useState<number | null>(null);

  // 1. Initialize WASM Bridge
  useEffect(() => {
    async function loadWasm() {
      try {
        setLoading(true);
        const b = await getRouteGuardBridge();
        setBridge(b);
        const g = b.getGraph();
        setGraph(g);

        // Run baseline route & state
        const r = b.findRoute(0, 10, false);
        setRouteResult(r);
        const bfs = b.runBFS(0, 10, false);
        setTraversalResult(bfs);
        const comp = b.getComponents(false);
        setComponentsResult(comp);
        const state = b.getNetworkState(0, 10, false);
        setNetworkState(state);
        const failTbl = b.getFailureTable(0, 10);
        setFailureTable(failTbl);

        setLoading(false);
      } catch (err: unknown) {
        console.error('Failed to initialize RouteGuard WASM module:', err);
        setErrorMsg('Failed to instantiate WebAssembly engine. Please check your browser WebAssembly support.');
        setLoading(false);
      }
    }
    loadWasm();
  }, []);

  // Refresh active computations from C engine
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

  // Handlers
  const handleToggleEdge = (edgeId: number) => {
    if (!bridge || !graph) return;
    const currentEdge = graph.edges.find((e) => e.id === edgeId);
    if (!currentEdge) return;

    bridge.setEdgeActive(edgeId, !currentEdge.active);
    recomputeAll(bridge, srcVertex, dstVertex, useMatrix);
  };

  const handleReset = () => {
    if (!bridge) return;
    bridge.reset();
    setSrcVertex(0);
    setDstVertex(10);
    recomputeAll(bridge, 0, 10, useMatrix);
  };

  const handleRunBFS = () => {
    if (!bridge) return;
    const res = bridge.runBFS(srcVertex, dstVertex, useMatrix);
    setTraversalResult(res);
    const r = bridge.findRoute(srcVertex, dstVertex, useMatrix);
    setRouteResult(r);
    setActiveTab('routing');
  };

  const handleRunDFS = () => {
    if (!bridge) return;
    const res = bridge.runDFS(srcVertex, dstVertex, useMatrix);
    setTraversalResult(res);
    setActiveTab('routing');
  };

  const handleFindRoute = () => {
    if (!bridge) return;
    const r = bridge.findRoute(srcVertex, dstVertex, useMatrix);
    setRouteResult(r);
    setActiveTab('routing');
  };

  const handleShowComponents = () => {
    if (!bridge) return;
    const comp = bridge.getComponents(useMatrix);
    setComponentsResult(comp);
    setActiveTab('routing');
  };

  const handleRepresentationChange = (matrix: boolean) => {
    setUseMatrix(matrix);
    if (bridge) {
      recomputeAll(bridge, srcVertex, dstVertex, matrix);
    }
  };

  const handleExportSVG = () => {
    if (!bridge) return;
    const svgStr = bridge.exportSVG(srcVertex, dstVertex, useMatrix);
    const blob = new Blob([svgStr], { type: 'image/svg+xml' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `routeguard_campus_src${srcVertex}_dst${dstVertex}.svg`;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
  };

  // Set of route edges for highlighting
  const routeEdgeMap = useMemo(() => {
    const map = new Map<string, number>();
    if (!routeResult || !routeResult.found || routeResult.path.length < 2) return map;

    for (let i = 0; i < routeResult.path.length - 1; i++) {
      const u = routeResult.path[i];
      const v = routeResult.path[i + 1];
      const key1 = `${u}-${v}`;
      const key2 = `${v}-${u}`;
      map.set(key1, i + 1);
      map.set(key2, i + 1);
    }
    return map;
  }, [routeResult]);

  // Set of reached vertices
  const reachedSet = useMemo(() => {
    const set = new Set<number>();
    if (traversalResult && traversalResult.order) {
      traversalResult.order.forEach((v) => set.add(v));
    } else {
      // Default all if no traversal
      if (graph) {
        for (let i = 0; i < graph.n_vertices; i++) set.add(i);
      }
    }
    return set;
  }, [traversalResult, graph]);

  if (loading) {
    return (
      <div className="app-container" style={{ justifyContent: 'center', alignItems: 'center', minHeight: '100vh' }}>
        <div style={{ textAlign: 'center' }}>
          <div style={{ fontSize: '24px', fontWeight: 800, color: 'var(--cyan)' }}>RouteGuard</div>
          <div style={{ color: 'var(--text-secondary)', marginTop: '8px' }}>Initializing ISO C11 WebAssembly Core Engine...</div>
        </div>
      </div>
    );
  }

  if (errorMsg) {
    return (
      <div className="app-container" style={{ justifyContent: 'center', alignItems: 'center', minHeight: '100vh' }}>
        <div style={{ background: 'rgba(239, 68, 68, 0.15)', border: '1px solid var(--rose)', padding: '24px', borderRadius: '8px', maxWidth: '500px' }}>
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
      {/* 1. Header */}
      <header className="app-header">
        <div className="brand-section">
          <div className="brand-icon">RG</div>
          <div>
            <div className="brand-title">
              RouteGuard
              <span className="brand-badge">WASM / C11</span>
            </div>
            <div className="brand-subtitle">Network Resilience &amp; Route Discovery Console</div>
          </div>
        </div>

        <div className="disclaimer-banner">
          <span style={{ fontSize: '16px' }}>⚠️</span>
          <span>
            <strong>SYNTHETIC EDUCATIONAL NETWORK:</strong> Pedagogical campus model for CSC2101/CSC2130. Route length = hops.
            Not for real-world navigation or municipal transit.
          </span>
        </div>

        <div className="header-actions">
          <button className="btn btn-secondary btn-sm" onClick={handleReset} id="btn-header-reset">
            ↺ Reset Graph
          </button>
          <button className="btn btn-primary btn-sm" onClick={handleExportSVG} id="btn-export-svg">
            ↓ Export SVG
          </button>
        </div>
      </header>

      {/* 2. Main Content Grid */}
      <main className="main-content">
        {/* Panel A: Left Control Panel */}
        <section className="panel-card control-panel" aria-label="Control Panel">
          <div className="card-title">
            <span>Network Controls</span>
            <span className={`status-pill ${routeResult?.found ? 'success' : 'error'}`}>
              {routeResult?.found ? 'Connected' : 'Route Blocked'}
            </span>
          </div>

          {/* Source / Destination Selectors */}
          <div className="control-group">
            <label className="control-label" htmlFor="src-select">
              <span>Source Vertex</span>
              <span style={{ color: 'var(--emerald)' }}>[SRC]</span>
            </label>
            <select
              id="src-select"
              className="form-select"
              value={srcVertex}
              onChange={(e) => {
                const val = Number(e.target.value);
                setSrcVertex(val);
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
              <span>Destination Vertex</span>
              <span style={{ color: 'var(--cyan)' }}>[DST]</span>
            </label>
            <select
              id="dst-select"
              className="form-select"
              value={dstVertex}
              onChange={(e) => {
                const val = Number(e.target.value);
                setDstVertex(val);
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

          {/* Representation Switch */}
          <div className="control-group">
            <label className="control-label">
              <span>Graph Representation</span>
              <span style={{ fontSize: '11px', color: 'var(--text-muted)' }}>
                {useMatrix ? 'O(V²) Scans' : 'O(V+E) Lists'}
              </span>
            </label>
            <div className="seg-switch">
              <button
                className={`seg-btn ${!useMatrix ? 'active' : ''}`}
                onClick={() => handleRepresentationChange(false)}
                id="btn-rep-list"
              >
                Adjacency List
              </button>
              <button
                className={`seg-btn ${useMatrix ? 'active' : ''}`}
                onClick={() => handleRepresentationChange(true)}
                id="btn-rep-matrix"
              >
                Adjacency Matrix
              </button>
            </div>
          </div>

          {/* Primary Traversal Actions */}
          <div className="control-group" style={{ gap: '8px' }}>
            <button className="btn btn-primary" onClick={handleRunBFS} id="btn-run-bfs">
              Run BFS Traversal
            </button>
            <button className="btn btn-secondary" onClick={handleRunDFS} id="btn-run-dfs">
              Run DFS Traversal
            </button>
            <button className="btn btn-secondary" onClick={handleFindRoute} id="btn-find-route">
              Find Minimum-Hop Route
            </button>
            <button className="btn btn-secondary" onClick={handleShowComponents} id="btn-show-components">
              Analyze Connected Components
            </button>
          </div>

          {/* Quick Scenario Buttons */}
          <div className="control-group">
            <label className="control-label">
              <span>Failure Scenarios</span>
            </label>
            <div style={{ display: 'flex', flexDirection: 'column', gap: '6px' }}>
              <button
                className={`btn btn-sm ${graph?.edges[9]?.active ? 'btn-danger' : 'btn-secondary'}`}
                onClick={() => handleToggleEdge(9)}
                id="btn-scenario-e9"
              >
                {graph?.edges[9]?.active ? 'Disable e9 (Critical Bridge Cut)' : 'Restore e9 (Reconnect Complex)'}
              </button>
              <button
                className={`btn btn-sm ${graph?.edges[5]?.active ? 'btn-danger' : 'btn-secondary'}`}
                onClick={() => handleToggleEdge(5)}
                id="btn-scenario-e5"
              >
                {graph?.edges[5]?.active ? 'Disable e5 (Redundant Cut: Block A)' : 'Restore e5 (Direct Plaza)'}
              </button>
              <button className="btn btn-secondary btn-sm" onClick={handleReset} id="btn-restore-all">
                Restore All Walkways (Reset)
              </button>
            </div>
          </div>

          {/* Quick Edge Mutation Dropdown */}
          <div className="control-group">
            <label className="control-label">
              <span>Toggle Specific Walkway</span>
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
                Select an edge to toggle...
              </option>
              {graph?.edges.map((e) => (
                <option key={e.id} value={e.id}>
                  e{e.id}: {graph.vertices[e.u]?.label} ↔ {graph.vertices[e.v]?.label} ({e.active ? 'ACTIVE' : 'CLOSED'})
                </option>
              ))}
            </select>
          </div>
        </section>

        {/* Panel B: Central Graph Canvas */}
        <section className="canvas-card" aria-label="Interactive Network Canvas">
          <div className="canvas-toolbar">
            <div style={{ display: 'flex', alignItems: 'center', gap: '12px' }}>
              <span style={{ fontWeight: 700, fontSize: '14px', textTransform: 'uppercase', letterSpacing: '0.5px' }}>
                Campus Walkway Topology
              </span>
              <span className="status-pill success">F1 Synthetic Fixture</span>
            </div>

            <div className="legend-bar">
              <div className="legend-item">
                <div className="legend-indicator active-edge"></div>
                <span>Active Walkway</span>
              </div>
              <div className="legend-item">
                <div className="legend-indicator closed-edge"></div>
                <span style={{ color: 'var(--rose)' }}>Closed (Dashed)</span>
              </div>
              <div className="legend-item">
                <div className="legend-indicator route-edge"></div>
                <span style={{ color: 'var(--cyan)' }}>Route Hops</span>
              </div>
              <div className="legend-item">
                <div className="legend-indicator src-node"></div>
                <span>Source</span>
              </div>
              <div className="legend-item">
                <div className="legend-indicator dst-node"></div>
                <span>Destination</span>
              </div>
            </div>
          </div>

          {/* Interactive SVG Network Map */}
          <div className="svg-canvas-wrapper">
            <svg
              className="svg-canvas"
              viewBox="0 0 1180 430"
              preserveAspectRatio="xMidYMid meet"
              xmlns="http://www.w3.org/2000/svg"
            >
              <defs>
                <filter id="glow-cyan" x="-20%" y="-20%" width="140%" height="140%">
                  <feGaussianBlur stdDeviation="3" result="blur" />
                  <feComposite in="SourceGraphic" in2="blur" operator="over" />
                </filter>
                <filter id="glow-red" x="-20%" y="-20%" width="140%" height="140%">
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
                    key={`edge-${e.id}`}
                    onClick={() => handleToggleEdge(e.id)}
                    onMouseEnter={() => setHoveredEdge(e.id)}
                    onMouseLeave={() => setHoveredEdge(null)}
                    style={{ cursor: 'pointer' }}
                  >
                    {/* Wider transparent hit-area */}
                    <line
                      x1={uNode.x}
                      y1={uNode.y}
                      x2={vNode.x}
                      y2={vNode.y}
                      stroke="transparent"
                      strokeWidth={16}
                    />

                    {/* Visible line */}
                    {!e.active ? (
                      /* Disabled / Closed edge */
                      <>
                        <line
                          x1={uNode.x}
                          y1={uNode.y}
                          x2={vNode.x}
                          y2={vNode.y}
                          stroke="#ef4444"
                          strokeWidth={2.5}
                          strokeDasharray="6,4"
                          filter={isHovered ? 'url(#glow-red)' : undefined}
                        />
                        {/* CLOSED text badge */}
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
                      /* Active Route Hop */
                      <>
                        <line
                          x1={uNode.x}
                          y1={uNode.y}
                          x2={vNode.x}
                          y2={vNode.y}
                          stroke="#38bdf8"
                          strokeWidth={isHovered ? 5 : 4}
                          filter="url(#glow-cyan)"
                        />
                        {/* Numbered Hop Badge */}
                        <circle cx={midX} cy={midY} r={10} fill="#0284c7" stroke="#38bdf8" strokeWidth={1.5} />
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
                      /* Standard active edge */
                      <line
                        x1={uNode.x}
                        y1={uNode.y}
                        x2={vNode.x}
                        y2={vNode.y}
                        stroke={isHovered ? '#94a3b8' : '#475569'}
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
                const isReached = reachedSet.has(v.id);
                const isHovered = hoveredVertex === v.id;

                let fillColor = '#1e293b';
                let strokeColor = '#64748b';
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
                  fillColor = '#111827';
                  strokeColor = '#475569';
                }

                return (
                  <g
                    key={`vertex-${v.id}`}
                    onClick={() => {
                      if (v.id !== srcVertex) {
                        setDstVertex(v.id);
                        if (bridge) recomputeAll(bridge, srcVertex, v.id, useMatrix);
                      }
                    }}
                    onContextMenu={(e) => {
                      e.preventDefault();
                      setSrcVertex(v.id);
                      if (bridge) recomputeAll(bridge, v.id, dstVertex, useMatrix);
                    }}
                    onMouseEnter={() => setHoveredVertex(v.id)}
                    onMouseLeave={() => setHoveredVertex(null)}
                    style={{ cursor: 'pointer' }}
                  >
                    {/* Selection halo */}
                    {(isSrc || isDst || isHovered) && (
                      <circle
                        cx={v.x}
                        cy={v.y}
                        r={24}
                        fill="none"
                        stroke={isSrc ? '#10b981' : isDst ? '#38bdf8' : '#94a3b8'}
                        strokeWidth={1}
                        strokeDasharray={isHovered ? '2,2' : undefined}
                        opacity={0.6}
                      />
                    )}

                    {/* Node circle */}
                    <circle
                      cx={v.x}
                      cy={v.y}
                      r={18}
                      fill={fillColor}
                      stroke={strokeColor}
                      strokeWidth={strokeWidth}
                      strokeDasharray={!isReached ? '4,3' : undefined}
                    />

                    {/* Vertex ID text */}
                    <text
                      x={v.x}
                      y={v.y + 4.5}
                      fill="#ffffff"
                      fontSize={12}
                      fontWeight="bold"
                      textAnchor="middle"
                      fontFamily="var(--font-mono)"
                    >
                      {v.id}
                    </text>

                    {/* Label Badge below vertex */}
                    <g transform={`translate(${v.x}, ${v.y + 24})`}>
                      <rect
                        x={-50}
                        y={0}
                        width={100}
                        height={16}
                        rx={3}
                        fill="rgba(15, 23, 42, 0.85)"
                        stroke="rgba(51, 65, 85, 0.8)"
                      />
                      <text
                        x={0}
                        y={12}
                        fill="#f8fafc"
                        fontSize={10}
                        fontWeight="600"
                        textAnchor="middle"
                        fontFamily="var(--font-sans)"
                      >
                        {v.label}
                      </text>
                    </g>

                    {/* Source / Destination / Unreached Badges */}
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

                    {!isReached && !isSrc && (
                      <g transform={`translate(${v.x}, ${v.y - 25})`}>
                        <rect x={-32} y={0} width={64} height={14} rx={3} fill="#334155" />
                        <text x={0} y={10.5} fill="#cbd5e1" fontSize={8} fontWeight="700" textAnchor="middle">
                          UNREACHED
                        </text>
                      </g>
                    )}
                  </g>
                );
              })}
            </svg>
          </div>

          {/* Quick instructions underneath canvas */}
          <div style={{ marginTop: '8px', fontSize: '11px', color: 'var(--text-muted)', display: 'flex', justifyContent: 'space-between' }}>
            <span>Click any walkway/edge to toggle closed/active state.</span>
            <span>Click vertex to set Destination; Right-click to set Source.</span>
          </div>
        </section>

        {/* Panel C: Right Results / Analysis Panel */}
        <section className="panel-card results-panel" aria-label="Results and Metrics">
          <div className="tab-bar">
            <button
              className={`tab-btn ${activeTab === 'routing' ? 'active' : ''}`}
              onClick={() => setActiveTab('routing')}
              id="tab-routing"
            >
              Route &amp; Traversals
            </button>
            <button
              className={`tab-btn ${activeTab === 'metrics' ? 'active' : ''}`}
              onClick={() => setActiveTab('metrics')}
              id="tab-metrics"
            >
              C Counters
            </button>
            <button
              className={`tab-btn ${activeTab === 'failureTable' ? 'active' : ''}`}
              onClick={() => setActiveTab('failureTable')}
              id="tab-failure-table"
            >
              Failure Table
            </button>
          </div>

          {/* Tab 1: Routing & Traversals */}
          {activeTab === 'routing' && (
            <div style={{ display: 'flex', flexDirection: 'column', gap: '14px' }}>
              {/* Route Summary Card */}
              <div
                style={{
                  background: routeResult?.found ? 'rgba(56, 189, 248, 0.08)' : 'rgba(239, 68, 68, 0.1)',
                  border: `1px solid ${routeResult?.found ? 'rgba(56, 189, 248, 0.3)' : 'rgba(239, 68, 68, 0.4)'}`,
                  borderRadius: '6px',
                  padding: '12px',
                }}
              >
                <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
                  <span style={{ fontSize: '13px', fontWeight: 700 }}>
                    {routeResult?.found ? 'Minimum-Hop Route' : 'Destination Unreachable'}
                  </span>
                  <span className={`status-pill ${routeResult?.found ? 'success' : 'error'}`}>
                    {routeResult?.found ? `${routeResult.hop_count} Hops` : 'No Route'}
                  </span>
                </div>

                {routeResult?.found ? (
                  <div style={{ marginTop: '8px' }}>
                    <div style={{ fontSize: '11px', color: 'var(--text-secondary)', marginBottom: '4px' }}>
                      Optimal Walkway Sequence (BFS Level-Order Tree):
                    </div>
                    <div className="sequence-container">
                      {routeResult.path.map((vId, idx) => (
                        <React.Fragment key={`route-hop-${vId}-${idx}`}>
                          <span className="seq-chip route-hop">
                            {vId}: {graph?.vertices[vId]?.label}
                          </span>
                          {idx < routeResult.path.length - 1 && (
                            <span style={{ color: 'var(--cyan)', fontSize: '12px' }}>→</span>
                          )}
                        </React.Fragment>
                      ))}
                    </div>
                  </div>
                ) : (
                  <div style={{ marginTop: '8px', fontSize: '12px', color: '#fca5a5' }}>
                    Critical bridge walkway disconnected. No path exists from vertex {srcVertex} ({graph?.vertices[srcVertex]?.label}) to vertex {dstVertex} ({graph?.vertices[dstVertex]?.label}).
                  </div>
                )}
              </div>

              {/* Traversal Order Card */}
              <div style={{ background: 'var(--bg-card)', border: '1px solid var(--border-subtle)', borderRadius: '6px', padding: '12px' }}>
                <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '6px' }}>
                  <span style={{ fontSize: '12px', fontWeight: 700, textTransform: 'uppercase', color: 'var(--text-secondary)' }}>
                    {traversalResult?.algorithm ?? 'BFS'} Traversal Sequence
                  </span>
                  <span style={{ fontSize: '11px', color: 'var(--cyan)' }}>
                    {traversalResult?.order_len ?? 0} Nodes Visited
                  </span>
                </div>
                <div className="sequence-container">
                  {traversalResult?.order?.map((vId, idx) => (
                    <React.Fragment key={`trav-${vId}-${idx}`}>
                      <span className="seq-chip">
                        {vId}: {graph?.vertices[vId]?.label}
                      </span>
                      {idx < (traversalResult.order.length - 1) && (
                        <span style={{ color: 'var(--text-muted)', fontSize: '11px' }}>,</span>
                      )}
                    </React.Fragment>
                  ))}
                </div>
              </div>

              {/* Components & Connectivity Status */}
              <div style={{ background: 'var(--bg-card)', border: '1px solid var(--border-subtle)', borderRadius: '6px', padding: '12px' }}>
                <div style={{ fontSize: '12px', fontWeight: 700, textTransform: 'uppercase', color: 'var(--text-secondary)', marginBottom: '8px' }}>
                  Network Connectivity State
                </div>
                <div className="metrics-grid">
                  <div className="metric-box">
                    <div className="metric-label">Components</div>
                    <div className="metric-value">{networkState?.component_count ?? 1}</div>
                    <div className="metric-sub">{networkState?.component_count === 1 ? 'Fully Connected' : 'Partitioned Graph'}</div>
                  </div>
                  <div className="metric-box">
                    <div className="metric-label">Reachable from SRC</div>
                    <div className="metric-value">{networkState?.reachable_src_count ?? 12} / {graph?.n_vertices ?? 12}</div>
                    <div className="metric-sub">Nodes in SRC partition</div>
                  </div>
                </div>
                {componentsResult && componentsResult.component_count > 1 && (
                  <div style={{ marginTop: '10px', fontSize: '11px', color: 'var(--text-secondary)' }}>
                    <span style={{ fontWeight: 600 }}>Component Breakdown:</span>
                    <div style={{ display: 'flex', flexWrap: 'wrap', gap: '4px', marginTop: '4px' }}>
                      {componentsResult.components.map((c) => (
                        <span key={`comp-node-${c.vertex}`} className="seq-chip">
                          Node {c.vertex}: Comp {c.component_id}
                        </span>
                      ))}
                    </div>
                  </div>
                )}
              </div>
            </div>
          )}

          {/* Tab 2: C Operation Counters */}
          {activeTab === 'metrics' && (
            <div style={{ display: 'flex', flexDirection: 'column', gap: '14px' }}>
              <div className="metrics-grid">
                <div className="metric-box">
                  <div className="metric-label">Slots Examined</div>
                  <div className="metric-value" style={{ color: useMatrix ? 'var(--amber)' : 'var(--emerald)' }}>
                    {traversalResult?.metrics.neighbor_slots_examined ?? 0}
                  </div>
                  <div className="metric-sub">
                    {useMatrix ? 'V² = 144 columns scanned' : '2E = 26 list nodes inspected'}
                  </div>
                </div>

                <div className="metric-box">
                  <div className="metric-label">Nodes Processed</div>
                  <div className="metric-value">{traversalResult?.metrics.vertices_processed ?? 0}</div>
                  <div className="metric-sub">Dequeued / Popped-not-stale</div>
                </div>

                <div className="metric-box">
                  <div className="metric-label">Inactive Skips</div>
                  <div className="metric-value">{traversalResult?.metrics.edges_skipped_inactive ?? 0}</div>
                  <div className="metric-sub">Disabled edges bypassed</div>
                </div>

                <div className="metric-box">
                  <div className="metric-label">Queue Ops (BFS)</div>
                  <div className="metric-value">
                    {traversalResult?.metrics.queue_enqueues ?? 0} / {traversalResult?.metrics.queue_dequeues ?? 0}
                  </div>
                  <div className="metric-sub">Enqueues / Dequeues</div>
                </div>

                <div className="metric-box">
                  <div className="metric-label">Stack Ops (DFS)</div>
                  <div className="metric-value">
                    {traversalResult?.metrics.stack_pushes ?? 0} / {traversalResult?.metrics.stack_pops ?? 0}
                  </div>
                  <div className="metric-sub">Pushes / Pops (Cap 2E+1)</div>
                </div>

                <div className="metric-box">
                  <div className="metric-label">Stale Pops (DFS)</div>
                  <div className="metric-value" style={{ color: 'var(--amber)' }}>
                    {traversalResult?.metrics.stale_pops ?? 0}
                  </div>
                  <div className="metric-sub">Re-entrant pops skipped</div>
                </div>
              </div>

              {/* Theoretical Complexity Comparison Note */}
              <div
                style={{
                  background: 'var(--bg-card)',
                  border: '1px solid var(--border-medium)',
                  borderRadius: '6px',
                  padding: '12px',
                  fontSize: '12px',
                  color: 'var(--text-secondary)',
                  lineHeight: '1.6',
                }}
              >
                <strong style={{ color: 'var(--text-main)', display: 'block', marginBottom: '4px' }}>
                  Algorithm Complexity Note (CSC2101 Lecture 37):
                </strong>
                {useMatrix ? (
                  <span>
                    <strong>Adjacency Matrix (Active):</strong> Storage is <span style={{ fontFamily: 'var(--font-mono)' }}>O(V²)</span> (144 ints).
                    Every vertex explores all <span style={{ fontFamily: 'var(--font-mono)' }}>V=12</span> columns in the matrix regardless of degree,
                    producing exactly 144 slot examinations on baseline F1.
                  </span>
                ) : (
                  <span>
                    <strong>Adjacency List (Active):</strong> Storage is <span style={{ fontFamily: 'var(--font-mono)' }}>O(V + E)</span> (12 heads + 26 nodes).
                    Traversal examines only existing incident neighbors (<span style={{ fontFamily: 'var(--font-mono)' }}>2E=26</span> nodes total),
                    achieving asymptotic optimality on sparse networks.
                  </span>
                )}
              </div>
            </div>
          )}

          {/* Tab 3: Single-Failure Impact Table */}
          {activeTab === 'failureTable' && (
            <div style={{ display: 'flex', flexDirection: 'column', gap: '10px' }}>
              <div style={{ fontSize: '12px', color: 'var(--text-secondary)' }}>
                Evaluates network resilience by simulating the closure of each walkway individually via brute-force repeated BFS (<span style={{ fontFamily: 'var(--font-mono)' }}>O(E · (V+E))</span>).
                Click any row to toggle that walkway.
              </div>

              <div className="data-table-container">
                <table className="data-table">
                  <thead>
                    <tr>
                      <th>Edge</th>
                      <th>Connection</th>
                      <th>Status</th>
                      <th>Route?</th>
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
                          key={`fail-row-${row.edge_id}`}
                          onClick={() => handleToggleEdge(row.edge_id)}
                          className={isCritical ? 'critical-cut' : ''}
                          style={{ cursor: 'pointer' }}
                        >
                          <td style={{ fontWeight: 700, color: 'var(--cyan)' }}>e{row.edge_id}</td>
                          <td style={{ fontSize: '11px', fontFamily: 'var(--font-sans)' }}>
                            {uLabel} ↔ {vLabel}
                          </td>
                          <td>
                            <span className={`status-pill ${isClosed ? 'error' : 'success'}`} style={{ fontSize: '10px' }}>
                              {isClosed ? 'CLOSED' : 'ACTIVE'}
                            </span>
                          </td>
                          <td>
                            <span style={{ color: row.reachable ? 'var(--emerald)' : 'var(--rose)', fontWeight: 700 }}>
                              {row.reachable ? 'YES' : 'NO (CUT)'}
                            </span>
                          </td>
                          <td>{row.hop_count === -1 ? '—' : row.hop_count}</td>
                          <td style={{ color: row.component_count > 1 ? 'var(--amber)' : 'inherit' }}>
                            {row.component_count}
                          </td>
                        </tr>
                      );
                    })}
                  </tbody>
                </table>
              </div>
            </div>
          )}

          {/* Footer Metrics */}
          <div
            style={{
              borderTop: '1px solid var(--border-subtle)',
              paddingTop: '10px',
              marginTop: 'auto',
              display: 'flex',
              justifyContent: 'space-between',
              alignItems: 'center',
              fontSize: '11px',
              color: 'var(--text-muted)',
            }}
          >
            <span>Active Walkways: {activeEdgeCount} / {graph?.n_edges ?? 13}</span>
            <span>Closed: {closedEdgeCount}</span>
            <span>Components: {networkState?.component_count ?? 1}</span>
          </div>
        </section>
      </main>
    </div>
  );
};

export default App;
