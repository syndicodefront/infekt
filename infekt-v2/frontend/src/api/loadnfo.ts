// command: load_nfo
export type LoadNfoRequest = {
  req: {
    filePath: string;
  }
}

export interface LoadNfoResponse {
  success: boolean;
  message: string | null;
}
