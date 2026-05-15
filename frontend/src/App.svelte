<script lang="ts">
  import { onMount } from 'svelte';

  import AppHeader from './lib/components/AppHeader.svelte';
  import FilterBar from './lib/components/FilterBar.svelte';
  import MediaSidebar from './lib/components/MediaSidebar.svelte';
  import MediaViewer from './lib/components/MediaViewer.svelte';
  import { fetchMedia, fetchMediaItem } from './lib/api';
  import type { MediaItem, MediaKind } from './lib/types';

  let items: MediaItem[] = [];
  let filteredItems: MediaItem[] = [];
  let selectedItem: MediaItem | null = null;
  let selectedId = '';
  let selectedKind: MediaKind | 'all' = 'all';
  let search = '';
  let searchDraft = '';
  let loadingList = true;
  let loadingItem = false;
  let listError = '';
  let viewerError = '';
  let initialized = false;

  function applyFilters(source: MediaItem[]): MediaItem[] {
    const normalizedQuery = search.trim().toLowerCase();
    return source.filter((item) => {
      return normalizedQuery.length === 0 || item.title.toLowerCase().includes(normalizedQuery);
    });
  }

  function syncUrl(): void {
    const url = new URL(window.location.href);
    if (selectedKind === 'all') {
      url.searchParams.delete('kind');
    } else {
      url.searchParams.set('kind', selectedKind);
    }

    if (search.trim().length > 0) {
      url.searchParams.set('q', search.trim());
    } else {
      url.searchParams.delete('q');
    }

    if (selectedId) {
      url.searchParams.set('id', selectedId);
    } else {
      url.searchParams.delete('id');
    }

    window.history.replaceState({}, '', url);
  }

  function readUrlState(): void {
    const params = new URLSearchParams(window.location.search);
    const kind = params.get('kind');
    const id = params.get('id');
    const q = params.get('q');

    if (kind === 'video' || kind === 'audio' || kind === 'image') {
      selectedKind = kind;
    }

    if (id) {
      selectedId = id;
    }

    if (q) {
      search = q;
      searchDraft = q;
    }
  }

  async function loadLibrary(kind: MediaKind | 'all'): Promise<void> {
    loadingList = true;
    listError = '';
    viewerError = '';

    try {
      items = await fetchMedia(kind);
      filteredItems = applyFilters(items);

      if (filteredItems.some((item) => item.id === selectedId)) {
        await loadSelectedItem(selectedId);
      } else if (filteredItems.length > 0) {
        selectedId = filteredItems[0].id;
        await loadSelectedItem(selectedId);
      } else {
        selectedId = '';
        selectedItem = null;
      }
    } catch (error) {
      listError = error instanceof Error ? error.message : 'Failed to load media';
    } finally {
      loadingList = false;
    }
  }

  async function loadSelectedItem(id: string): Promise<void> {
    if (!id) {
      selectedItem = null;
      return;
    }

    loadingItem = true;
    viewerError = '';
    try {
      selectedItem = await fetchMediaItem(id);
    } catch (error) {
      viewerError = error instanceof Error ? error.message : 'Failed to load item';
    } finally {
      loadingItem = false;
    }
  }

  $: filteredItems = applyFilters(items);

  $: if (initialized && filteredItems.length === 0) {
    selectedId = '';
    selectedItem = null;
  } else if (initialized && filteredItems.length > 0 && !filteredItems.some((item) => item.id === selectedId)) {
    selectedId = filteredItems[0].id;
  }

  $: if (initialized && selectedId && (!selectedItem || selectedItem.id !== selectedId)) {
    loadSelectedItem(selectedId);
  }

  $: if (initialized) {
    syncUrl();
  }

  onMount(async () => {
    readUrlState();
    initialized = true;
    await loadLibrary(selectedKind);
  });
</script>

<svelte:head>
  <title>MediaBrowser</title>
  <meta
    name="description"
    content="Browse your local media library through the Drogon API with a Svelte frontend."
  />
</svelte:head>

<div class="app-shell">
  <AppHeader totalItems={items.length} />

  <section class="toolbar-card">
    <FilterBar
      {selectedKind}
      on:kindchange={async (event) => {
        selectedKind = event.detail;
        await loadLibrary(selectedKind);
      }}
    />

    <form class="search-form" method="GET" action="/">
      {#if selectedKind !== 'all'}
        <input type="hidden" name="kind" value={selectedKind} />
      {/if}
      <label class="search-box">
        <span>Search</span>
        <input bind:value={searchDraft} name="q" placeholder="Find by title..." />
      </label>
      <button class="search-button" type="submit">Search</button>
    </form>
  </section>

  {#if listError}
    <div class="banner error">{listError}</div>
  {:else if loadingList}
    <div class="banner">Loading media library...</div>
  {:else}
    <main class="content-grid">
      <MediaSidebar
        items={filteredItems}
        {selectedId}
        on:select={(event) => {
          selectedId = event.detail;
        }}
      />
      <MediaViewer item={selectedItem} loading={loadingItem} error={viewerError} />
    </main>
  {/if}
</div>

<style>
  .app-shell {
    width: min(1320px, calc(100vw - 2rem));
    margin: 0 auto;
    padding: 1rem 0 2rem;
    display: grid;
    gap: 1rem;
  }

  .toolbar-card {
    background: rgba(255, 255, 255, 0.68);
    border: 1px solid rgba(18, 63, 108, 0.08);
    border-radius: 24px;
    padding: 1rem;
    box-shadow: 0 16px 42px rgba(31, 62, 99, 0.08);
    display: grid;
    gap: 1rem;
  }

  .search-form {
    display: grid;
    grid-template-columns: minmax(0, 1fr) auto;
    gap: 0.75rem;
    align-items: end;
  }

  .search-box {
    display: grid;
    gap: 0.45rem;
    color: #3d5367;
  }

  .search-box input {
    border: 1px solid rgba(18, 63, 108, 0.12);
    border-radius: 14px;
    padding: 0.78rem 0.95rem;
    background: rgba(255, 255, 255, 0.82);
  }

  .search-box input:focus {
    outline: 2px solid rgba(16, 112, 202, 0.25);
    border-color: rgba(16, 112, 202, 0.35);
  }

  .search-button {
    border: none;
    border-radius: 14px;
    padding: 0.78rem 1rem;
    background: #123f6c;
    color: white;
    min-width: 110px;
    font: inherit;
    cursor: pointer;
  }

  .content-grid {
    display: grid;
    grid-template-columns: minmax(280px, 360px) minmax(0, 1fr);
    gap: 1rem;
    align-items: start;
  }

  .banner {
    padding: 1rem 1.2rem;
    border-radius: 20px;
    background: rgba(255, 255, 255, 0.78);
    border: 1px solid rgba(18, 63, 108, 0.08);
    color: #30516f;
  }

  .banner.error {
    background: #fff2f2;
    color: #8c2f2f;
  }

  @media (max-width: 920px) {
    .content-grid {
      grid-template-columns: 1fr;
    }
  }

  @media (max-width: 640px) {
    .search-form {
      grid-template-columns: 1fr;
    }
  }
</style>
