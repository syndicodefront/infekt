export interface NfoRendererGrid {
  width: number;
  height: number;
  lines: NfoRendererLine[];
  hasBlocks: boolean;
}

export interface NfoRendererLine {
  row: number;
  blockGroups: NfoRendererBlockGroup[];
  textFlights: NfoRendererTextFlight[];
  links: NfoRendererLink[];
}

export interface NfoRendererBlockGroup {
  col: number;
  blocks: NfoRendererBlockShape[];
}

export enum NfoRendererBlockShape {
  NoBlock = 0,
  Whitespace,
  WhitespaceInText,
  FullBlock,
  FullBlockLightShade,
  FullBlockMediumShade,
  FullBlockDarkShade,
  LowerHalf,
  UpperHalf,
  RightHalf,
  LeftHalf,
  BlackSquare,
  BlackSquareSmall,
}

export interface NfoRendererTextFlight {
  col: number;
  text: string;
}

export interface NfoRendererLink {
  col: number;
  text: string;
  url: string;
}
