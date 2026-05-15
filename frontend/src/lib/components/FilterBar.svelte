<script lang="ts">
  import { createEventDispatcher } from 'svelte';
  import type { MediaKind } from '../types';

  export let selectedKind: MediaKind | 'all' = 'all';

  const dispatch = createEventDispatcher<{
    kindchange: MediaKind | 'all';
  }>();

  const filters: Array<{ label: string; value: MediaKind | 'all' }> = [
    { label: 'All', value: 'all' },
    { label: 'Videos', value: 'video' },
    { label: 'Audio', value: 'audio' },
    { label: 'Images', value: 'image' }
  ];

</script>

<div class="filters">
  <div class="chip-row">
    {#each filters as filter}
      <button
        class:selected={filter.value === selectedKind}
        type="button"
        on:click={() => dispatch('kindchange', filter.value)}
      >
        {filter.label}
      </button>
    {/each}
  </div>
</div>

<style>
  .filters {
    display: flex;
    gap: 1rem;
    align-items: center;
    flex-wrap: wrap;
  }

  .chip-row {
    display: flex;
    gap: 0.6rem;
    flex-wrap: wrap;
  }

  button {
    border: none;
    border-radius: 999px;
    padding: 0.72rem 1rem;
    background: #dfeaf5;
    color: #20415d;
    transition: transform 140ms ease, background-color 140ms ease;
  }

  button.selected {
    background: #123f6c;
    color: white;
  }

  button:hover {
    transform: translateY(-1px);
  }

</style>
