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

See [the API documentation](./higan/target-web/api.d.ts) to learn how
to connect controllers and so on! You can also have a look at the 
git repository's app code to see a relatively complete implementation example.

> Installing the library

```shell
npm install byuu
```

The package contains a [TypeScript](https://www.typescriptlang.org/) type definition where the 
documentation of the [available API](./higan/target-web/api.d.ts) can be found.

> ./index.js

```js
import byuu from 'byuu'
const romPath = '/path/to/rom.sfc'
const container = document.body

// Initialization is only needed once
byuu.initialize(container, 800, 600)
  .then(() => byuu.loadURL(romPath))
  .then((romInfo) => {
    console.log(`Loaded ${romPath}`, romInfo)
    if (byuu.start()) {
      console.log('Emulator started successfully!')
    }
  })
```

When using with Webpack, you will be required to use the 
[file-loader](https://webpack.js.org/loaders/file-loader/) loader
to allow this library to properly load its WASM code.

> webpack.config.js

```js
module.exports = ( env, options ) => {
  // ...
  module: {
    rules: [
      { 
        test: /byuu-web-lib.wasm$/,
        type: 'javascript/auto',
        loader: 'file-loader',
        options: {
          name: '[name]-[hash].[ext]',         
        }
      }
    ]
  }
}
```

Here below is an example using [Vue.js](https://vuejs.org/). Although untested, it should be possible
to use this library similarly with other frameworks.

> vue.config.js

```js
module.exports = {
  configureWebpack: {
    module: {
      rules: [
        { 
          test: /byuu-web-lib.wasm$/,
          type: 'javascript/auto',
          loader: 'file-loader',
          options: {
            name: '[name]-[hash].[ext]',         
          }
        }
      ]
    }
  }
}
```

> ./src/components/HelloByuu.vue

```vue
<template>
  <div ref="container"></div>
</template>
<script>
import byuu from 'byuu'

export default {
  async mounted () {
    await byuu.initialize(this.$refs.container, 800, 600)
    await byuu.loadURL('/path/to/rom.sfc')
    byuu.start()
  },
  beforeDestroy() {
    byuu.terminate()
  }
}
</script>
<style scoped></style>
```

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

We would like to give our sincere thanks to:

- byuu for his contributions, help, and support
- [hex-usr](https://gitlab.com/hex-usr), maintainer of the [nSide](https://gitlab.com/hex-usr/nSide) fork
  (from which we have adapted some of the FC mappers)
- [Andrew Reading](https://gitlab.com/areading), maintainer of [bsnes-mcfly](https://gitlab.com/areading/bsnes-mcfly)
  (which we've looked into for ideas on optimization)