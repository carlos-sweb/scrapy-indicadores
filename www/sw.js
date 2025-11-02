const CACHE_NAME = 'indicadores-cache-v1';
const urlsToCache = [
  '/scrapy-indicadores/', // Ruta de inicio
  '/scrapy-indicadores/index.html',
  '/scrapy-indicadores/styles.css',
  '/scrapy-indicadores/manifest.json',
  '/scrapy-indicadores/icon-192x192.png',
  '/scrapy-indicadores/icon-512x512.png', // Archivo CSS
  // ... lista de otros archivos JS, imágenes y fuentes que quieres cachear
];

// Instalación: Cacha los archivos estáticos necesarios
self.addEventListener('install', event => {
  event.waitUntil(
    caches.open(CACHE_NAME)
      .then(cache => {
        console.log('Opened cache');
        return cache.addAll(urlsToCache);
      })
  );
});

// Fetch: Sirve contenido desde la caché o va a la red
self.addEventListener('fetch', event => {
  event.respondWith(
    caches.match(event.request)
      .then(response => {
        // Devuelve el recurso cacheado si existe
        if (response) {
          return response;
        }
        // Si no está cacheado, ve a la red
        return fetch(event.request);
      })
  );
});