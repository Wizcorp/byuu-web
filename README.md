# byuu-web

![byuu logo](https://byuu.org/images/byuu/github/byuu-logo-small.png)

**Status**: Dark launch (under heavy development)

byuu is a multi-system emulator that aims to combine the accuracy of
[higan](https://byuu.org/higan) with the simplicity and performance of
[bsnes](https://byuu.org/bsnes).

This repository attempts to bring the goodness of the original project to the Web. 
We make significant changes to the repository due to WebAssembly's behavior and 
performance restrictions, but try to stick to accuracy as much as possible. 
We also make sure to keep up-to-date with upstream, and as much as possible 
replicate the same internal behaviors when writing alternative implementations.

## Usage

### Application 

Go to https://wizcorp.github.io/byuu-web and enjoy!

### Library

> Installing the library

```shell
npm install byuu
```

The package contains a [TypeScript](https://www.typescriptlang.org/) type definition where the 
documentation of the [available API](./higan/target-web/api.d.ts) can be found.

You will need to make sure that a canvas DOM element with the id set to `canvas` has
been loaded on the page before you initialize the module.

> HTML file

```html
<canvas id="canvas" oncontextmenu="event.preventDefault()"></canvas>
```

Code 

> Example code

```js
const emulator = require('byuu')
const romPath = '/path/to/rom.sfc'

// Initialization is only needed once
emulator.init(800, 600)
  .then(() => emulator.loadURL(romPath))
  .then((romInfo) => {
    console.log(`Loaded ${romPath}`, romInfo)
    if (emulator.start()) {
      console.log('Emulator started successfully!')
    }
  })
```

See [the API documentation](./higan/target-web/api.d.ts) to learn how
to connect controllers and so on! You can also have a look at the 
git repository's app code to see a more complete implementation example.

## Development

Git is used for the development of new releases, and represents a staging
environment. As byuu is rather mature, things should generally be quite stable.
However, bugs will exist, regressions will occur, so proceed at your own risk.

> Getting started

```shell
git clone [...] byuu-web
cd byuu-web
make all debug=true # debug is optional but strongly recommended for development
# You can also build the package and the app individually
make package
make app
```

To achieve acceptable performances, byuu-web changes by default how the scheduler
is implemented and how each `Thread`'s main method behaves. This can be reverted
to test against upstream:

> Using the standard scheduler

```shell
make debug=true synchro=false
```

Once you have a build, start the HTTP development server.

> Serving the emulator

```shell
make serve
```

> Cleaning builds

```shell
make clean
```

The emulator will be served at http://localhost:8000.

## Links

  - [Official website](https://byuu.org/byuu)
  - [Upstream git repository](https://github.com/byuu/byuu)
  - [Donations](https://patreon.com/byuu)

## Acknowledgements

We would like to give our sincere thanks to byuu for his contributions, help,
and support.
