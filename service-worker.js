const currentVersion = '9fd17a7';

self.addEventListener('fetch', (event) => {
    event.respondWith(
        caches.match(event.request).then((resp) => {
            return resp || fetch(event.request).then((response) => {
                return caches.open(currentVersion).then((cache) => {
                    cache.put(event.request, response.clone());
                    return response;
                });  
            });
        })
    );
});

self.addEventListener('install', (event) => {
    event.waitUntil(
        caches.open(currentVersion)
            .then((cache) => {
                return cache.addAll([
                    './index.html',
                    './byuu-web.js',
                    './byuu-web.wasm',
                    './assets/icons/favicon-32x32.png',
                    './assets/icons/favicon-16x16.png',
                    './assets/icons/mstile-150x150.png',
                    './assets/icons/android-chrome-512x512.png',
                    './assets/icons/android-chrome-192x192.png',
                    './assets/icons/favicon.ico',
                    './assets/icons/browserconfig.xml',
                    './assets/icons/apple-touch-icon.png',
                    './assets/icons/safari-pinned-tab.svg',
                    './assets/images/menu.png',
                    './assets/images/arrowDown.png',
                    './assets/images/gamepad/buttonTriangleLeft.png',
                    './assets/images/gamepad/buttonTriangleDown.png',
                    './assets/images/gamepad/buttonTriangleRight.png',
                    './assets/images/gamepad/buttonTriangleUp.png',
                    './assets/images/gamepad/buttonCircle.png',
                    './assets/images/gamepad/buttonRectangle.png',
                ]);
            })
    );
});

self.addEventListener('activate', (event) => {
    var cacheKeeplist = [currentVersion];

    event.waitUntil(
        caches.keys().then((keyList) => {
            return Promise.all(keyList.map((key) => {
                if (cacheKeeplist.indexOf(key) === -1) {
                    return caches.delete(key);
                }
            }));
        })
    );
});
