export interface RouteGuardModule {
    _rg_wasm_init(): number;
    _rg_wasm_reset(): number;
    _rg_wasm_get_graph_json(): number;
    _rg_wasm_set_edge_active(edge_id: number, active: number): number;
    _rg_wasm_run_bfs(src: number, dst: number, use_matrix: number): number;
    _rg_wasm_run_dfs(src: number, dst: number, use_matrix: number): number;
    _rg_wasm_find_route(src: number, dst: number, use_matrix: number): number;
    _rg_wasm_get_components(use_matrix: number): number;
    _rg_wasm_get_network_state(src: number, dst: number, use_matrix: number): number;
    _rg_wasm_get_failure_table(src: number, dst: number): number;
    _rg_wasm_export_svg(src: number, dst: number, use_matrix: number): number;
    UTF8ToString(ptr: number): string;
}

export default function createRouteGuardModule(): Promise<RouteGuardModule>;
