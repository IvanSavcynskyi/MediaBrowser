<script lang="ts">
  import { createEventDispatcher } from 'svelte';
  import type { MediaItem } from '../types';

  export let items: MediaItem[] = [];
  export let selectedId = '';

  const dispatch = createEventDispatcher<{
    select: string;
  }>();

  function formatKind(kind: MediaItem['kind']): string {
    if (kind === 'video') return 'Video';
    if (kind === 'audio') return 'Audio';
    return 'Image';
  }
</script>

<aside class="sidebar">
  <div class="sidebar-header">
    <h2>Library</h2>
    <span>{items.length} items</span>
  </div>

  {#if items.length === 0}
    <div class="empty-state">No media matched the current filter.</div>
  {:else}
    <div class="item-list">
      {#each items as item}
        <button
          type="button"
          class="media-item"
          class:active={item.id === selectedId}
          on:click={() => dispatch('select', item.id)}
        >
          <span class="kind-tag">{formatKind(item.kind)}</span>
          <strong>{item.title}</strong>
          <small>{item.mime}</small>
        </button>
      {/each}
    </div>
  {/if}
</aside>

<style>
  .sidebar {
    min-height: 480px;
    background: rgba(255, 255, 255, 0.76);
    border: 1px solid rgba(18, 63, 108, 0.08);
    border-radius: 24px;
    padding: 1rem;
    box-shadow: 0 18px 48px rgba(31, 62, 99, 0.08);
  }

  .sidebar-header {
    display: flex;
    justify-content: space-between;
    gap: 1rem;
    align-items: baseline;
    margin-bottom: 0.9rem;
  }

  .sidebar-header h2 {
    margin: 0;
    font-size: 1.15rem;
  }

  .sidebar-header span {
    color: #557089;
    font-size: 0.95rem;
  }

  .item-list {
    display: grid;
    gap: 0.75rem;
  }

  .media-item {
    width: 100%;
    text-align: left;
    border: 1px solid rgba(18, 63, 108, 0.08);
    border-radius: 18px;
    padding: 0.95rem;
    background: #f9fbfd;
    display: grid;
    gap: 0.25rem;
    color: #1f3448;
  }

  .media-item.active {
    background: linear-gradient(135deg, #184f86, #0d2f58);
    color: white;
    border-color: transparent;
  }

  .kind-tag {
    display: inline-flex;
    width: fit-content;
    padding: 0.2rem 0.55rem;
    border-radius: 999px;
    background: rgba(24, 79, 134, 0.1);
    color: #184f86;
    font-size: 0.72rem;
    text-transform: uppercase;
    letter-spacing: 0.08em;
  }

  .media-item.active .kind-tag {
    background: rgba(255, 255, 255, 0.18);
    color: white;
  }

  strong {
    font-size: 1rem;
  }

  small {
    color: inherit;
    opacity: 0.72;
  }

  .empty-state {
    padding: 1rem;
    border-radius: 18px;
    background: #f5f8fb;
    color: #587189;
  }
</style>
