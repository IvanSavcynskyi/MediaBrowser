<script lang="ts">
  import type { MediaItem } from '../types';

  export let item: MediaItem | null = null;
  export let loading = false;
  export let error = '';

  function formatBytes(bytes: number): string {
    const units = ['B', 'KB', 'MB', 'GB'];
    let value = bytes;
    let unitIndex = 0;

    while (value >= 1024 && unitIndex < units.length - 1) {
      value /= 1024;
      unitIndex += 1;
    }

    return `${value.toFixed(unitIndex === 0 ? 0 : 1)} ${units[unitIndex]}`;
  }
</script>

<section class="viewer">
  {#if loading}
    <div class="placeholder">Loading media details...</div>
  {:else if error}
    <div class="placeholder error">{error}</div>
  {:else if !item}
    <div class="placeholder">Pick an item from the library to preview it here.</div>
  {:else}
    <div class="meta-row">
      <div>
        <span class="label">{item.kind}</span>
        <h2>{item.title}</h2>
      </div>
      <div class="meta-grid">
        <span>{item.mime}</span>
        <span>{formatBytes(item.size)}</span>
      </div>
    </div>

    <div class="preview-frame">
      {#if item.kind === 'image'}
        <img src={item.url} alt={item.title} />
      {:else if item.kind === 'video'}
        <!-- svelte-ignore a11y_media_has_caption -->
        <video controls preload="metadata" src={item.url}></video>
      {:else if item.kind === 'audio'}
        <div class="audio-shell">
          <div class="audio-art">AUDIO</div>
          <audio controls preload="metadata" src={item.url}></audio>
        </div>
      {/if}
    </div>
  {/if}
</section>

<style>
  .viewer {
    min-height: 480px;
    background: rgba(255, 255, 255, 0.86);
    border: 1px solid rgba(18, 63, 108, 0.08);
    border-radius: 28px;
    padding: 1.2rem;
    box-shadow: 0 20px 52px rgba(31, 62, 99, 0.08);
  }

  .placeholder {
    display: grid;
    place-items: center;
    min-height: 100%;
    border-radius: 22px;
    background: linear-gradient(180deg, #f7fafc, #edf3f8);
    color: #557089;
    padding: 2rem;
    text-align: center;
  }

  .placeholder.error {
    color: #922f2f;
    background: linear-gradient(180deg, #fff2f2, #ffe4e4);
  }

  .meta-row {
    display: flex;
    justify-content: space-between;
    gap: 1rem;
    align-items: start;
    margin-bottom: 1rem;
  }

  .label {
    display: inline-flex;
    padding: 0.24rem 0.6rem;
    border-radius: 999px;
    background: #ddeaf6;
    color: #224663;
    text-transform: uppercase;
    font-size: 0.72rem;
    letter-spacing: 0.08em;
  }

  h2 {
    margin: 0.65rem 0 0;
    font-size: clamp(1.5rem, 2.4vw, 2.2rem);
    color: #10253a;
  }

  .meta-grid {
    display: grid;
    gap: 0.4rem;
    justify-items: end;
    color: #5d7488;
  }

  .preview-frame {
    min-height: 380px;
    border-radius: 22px;
    background: linear-gradient(180deg, #0c2036, #142d49);
    display: grid;
    place-items: center;
    padding: 1rem;
    overflow: hidden;
  }

  img,
  video {
    width: 100%;
    max-height: 72vh;
    border-radius: 16px;
    object-fit: contain;
    background: #081522;
  }

  .audio-shell {
    width: min(100%, 680px);
    display: grid;
    gap: 1.4rem;
    justify-items: center;
  }

  .audio-art {
    width: min(100%, 360px);
    aspect-ratio: 1;
    border-radius: 28px;
    display: grid;
    place-items: center;
    background:
      radial-gradient(circle at top left, rgba(255, 209, 102, 0.82), transparent 36%),
      radial-gradient(circle at bottom right, rgba(41, 171, 226, 0.6), transparent 34%),
      linear-gradient(135deg, #1e5b93, #0d2f58);
    color: white;
    font-size: 1.35rem;
    letter-spacing: 0.2em;
  }

  audio {
    width: min(100%, 560px);
  }

  @media (max-width: 720px) {
    .meta-row {
      flex-direction: column;
    }

    .meta-grid {
      justify-items: start;
    }
  }
</style>
