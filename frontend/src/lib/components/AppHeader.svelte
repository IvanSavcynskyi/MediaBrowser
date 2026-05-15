<script lang="ts">
  export let totalItems = 0;
</script>

<header class="hero">
  <div class="eyebrow">MediaBrowser</div>
  <div class="headline-row">
    <div>
      <h1>Керуйте локальною медіатекою в одному вікні.</h1>
      <p>Усі відео, аудіо та зображення в одному зручному інтерфейсі.</p>
    </div>
    <div class="stat-card">
      <span>Файлів у бібліотеці</span>
      <strong>{totalItems}</strong>
    </div>
  </div>
</header>

<style>
  .hero {
    background: linear-gradient(135deg, rgba(16, 112, 202, 0.92), rgba(16, 59, 106, 0.96));
    color: #f6fbff;
    border-radius: 28px;
    padding: 1.6rem;
    box-shadow: 0 28px 60px rgba(13, 47, 88, 0.18);
  }

  .eyebrow {
    display: inline-flex;
    padding: 0.35rem 0.75rem;
    border-radius: 999px;
    background: rgba(255, 255, 255, 0.16);
    font-size: 0.84rem;
    letter-spacing: 0.08em;
    text-transform: uppercase;
  }

  .headline-row {
    display: flex;
    justify-content: space-between;
    gap: 1rem;
    margin-top: 1rem;
  }

  h1 {
    margin: 0 0 0.75rem;
    font-size: clamp(2rem, 5vw, 3.2rem);
    line-height: 0.98;
    max-width: 12ch;
  }

  p {
    margin: 0;
    max-width: 56ch;
    color: rgba(246, 251, 255, 0.85);
  }

  .stat-card {
    min-width: 160px;
    align-self: flex-start;
    padding: 1rem 1.1rem;
    border-radius: 22px;
    background: rgba(255, 255, 255, 0.12);
    backdrop-filter: blur(8px);
  }

  .stat-card span {
    display: block;
    font-size: 0.85rem;
    color: rgba(246, 251, 255, 0.7);
    margin-bottom: 0.45rem;
  }

  .stat-card strong {
    font-size: 2rem;
  }

  @media (max-width: 720px) {
    .headline-row {
      flex-direction: column;
    }

    .stat-card {
      min-width: 0;
    }
  }
</style>
