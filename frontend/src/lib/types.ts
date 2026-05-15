export type MediaKind = 'video' | 'audio' | 'image';

export interface MediaItem {
  id: string;
  title: string;
  kind: MediaKind;
  mime: string;
  size: number;
  url: string;
}

export interface MediaListResponse {
  count: number;
  items: MediaItem[];
}
