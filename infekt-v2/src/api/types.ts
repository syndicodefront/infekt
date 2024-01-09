// command: load_nfo
export type LoadNfoRequest = {
  req: {
    filePath: string;
  }
}

export type LoadNfoResponse = {
  success: boolean;
  message: string | null;
}
