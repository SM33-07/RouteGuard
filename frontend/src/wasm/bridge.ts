import createRouteGuardModule, { type RouteGuardModule } from './routeguard.js';

export interface Vertex {
  id: number;
  label: string;
  x: number;
  y: number;
}

export interface Edge {
  id: number;
  u: number;
  v: number;
  active: boolean;
}

export interface GraphData {
  n_vertices: number;
  n_edges: number;
  vertices: Vertex[];
  edges: Edge[];
}

export interface Metrics {
  vertices_processed: number;
  neighbor_slots_examined: number;
  edges_skipped_inactive: number;
  queue_enqueues: number;
  queue_dequeues: number;
  stack_pushes: number;
  stack_pops: number;
  stale_pops: number;
  elapsed_ms: number;
}

export interface TraversalResult {
  algorithm: 'BFS' | 'DFS';
  representation: string;
  src: number;
  dst: number;
  reachable: boolean;
  dist?: number;
  found?: boolean;
  hop_count?: number;
  order_len: number;
  order: number[];
  route?: number[];
  metrics: Metrics;
}

export interface RouteResult {
  src: number;
  dst: number;
  found: boolean;
  hop_count: number;
  path: number[];
  metrics: Metrics;
}

export interface ComponentsResult {
  component_count: number;
  components: Array<{
    vertex: number;
    component_id: number;
  }>;
}

export interface NetworkState {
  reachable_src_count: number;
  component_count: number;
  route_hops: number;
  route_found: boolean;
}

export interface FailureRow {
  edge_id: number;
  u: number;
  v: number;
  reachable: boolean;
  hop_count: number;
  reachable_src_count: number;
  component_count: number;
}

export interface FailureTableResult {
  src: number;
  dst: number;
  n_rows: number;
  rows: FailureRow[];
}

export class RouteGuardBridge {
  private mod: RouteGuardModule;

  constructor(mod: RouteGuardModule) {
    this.mod = mod;
  }

  public init(): boolean {
    const res = this.mod._rg_wasm_init();
    return res === 0;
  }

  public reset(): boolean {
    const res = this.mod._rg_wasm_reset();
    return res === 0;
  }

  public getGraph(): GraphData {
    const ptr = this.mod._rg_wasm_get_graph_json();
    const str = this.mod.UTF8ToString(ptr);
    return JSON.parse(str) as GraphData;
  }

  public setEdgeActive(edgeId: number, active: boolean): boolean {
    const res = this.mod._rg_wasm_set_edge_active(edgeId, active ? 1 : 0);
    return res === 0;
  }

  public runBFS(src: number, dst: number, useMatrix: boolean): TraversalResult {
    const ptr = this.mod._rg_wasm_run_bfs(src, dst, useMatrix ? 1 : 0);
    const str = this.mod.UTF8ToString(ptr);
    return JSON.parse(str) as TraversalResult;
  }

  public runDFS(src: number, dst: number, useMatrix: boolean): TraversalResult {
    const ptr = this.mod._rg_wasm_run_dfs(src, dst, useMatrix ? 1 : 0);
    const str = this.mod.UTF8ToString(ptr);
    return JSON.parse(str) as TraversalResult;
  }

  public findRoute(src: number, dst: number, useMatrix: boolean): RouteResult {
    const ptr = this.mod._rg_wasm_find_route(src, dst, useMatrix ? 1 : 0);
    const str = this.mod.UTF8ToString(ptr);
    return JSON.parse(str) as RouteResult;
  }

  public getComponents(useMatrix: boolean): ComponentsResult {
    const ptr = this.mod._rg_wasm_get_components(useMatrix ? 1 : 0);
    const str = this.mod.UTF8ToString(ptr);
    return JSON.parse(str) as ComponentsResult;
  }

  public getNetworkState(src: number, dst: number, useMatrix: boolean): NetworkState {
    const ptr = this.mod._rg_wasm_get_network_state(src, dst, useMatrix ? 1 : 0);
    const str = this.mod.UTF8ToString(ptr);
    return JSON.parse(str) as NetworkState;
  }

  public getFailureTable(src: number, dst: number): FailureTableResult {
    const ptr = this.mod._rg_wasm_get_failure_table(src, dst);
    const str = this.mod.UTF8ToString(ptr);
    return JSON.parse(str) as FailureTableResult;
  }

  public exportSVG(src: number, dst: number, useMatrix: boolean): string {
    const ptr = this.mod._rg_wasm_export_svg(src, dst, useMatrix ? 1 : 0);
    return this.mod.UTF8ToString(ptr);
  }
}

let bridgeInstance: RouteGuardBridge | null = null;

export async function getRouteGuardBridge(): Promise<RouteGuardBridge> {
  if (bridgeInstance) {
    return bridgeInstance;
  }
  const mod = await createRouteGuardModule();
  mod._rg_wasm_init();
  bridgeInstance = new RouteGuardBridge(mod);
  return bridgeInstance;
}
