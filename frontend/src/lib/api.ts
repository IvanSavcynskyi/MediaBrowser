import type { MediaItem, MediaKind, MediaListResponse } from './types';

async function handleJson<T>(response: Response): Promise<T> {
  if (!response.ok) {
    const message = `API request failed with status ${response.status}`;
    throw new Error(message);
  }
  return response.json() as Promise<T>;
}

export async function fetchMedia(kind: MediaKind | 'all', query = ''): Promise<MediaItem[]> {
  const params = new URLSearchParams();
  if (kind !== 'all') {
    params.set('kind', kind);
  }
  if (query.trim().length > 0) {
    params.set('q', query.trim());
  }

  const url = params.size > 0 ? `/api/media?${params.toString()}` : '/api/media';
  const payload = await handleJson<MediaListResponse>(await fetch(url));
  return payload.items;
}

export async function fetchMediaItem(id: string): Promise<MediaItem> {
  return handleJson<MediaItem>(await fetch(`/api/media/${id}`));
}
