// command: load_nfo
export type LoadNfoRequest = {
  req: {
    filePath: string;
    returnBrowseableFiles: boolean;
  }
}

export type LoadNfoResponse = {
  success: boolean;
  message: string | null;
  browseableFilePaths: Array<string>|null;
}
