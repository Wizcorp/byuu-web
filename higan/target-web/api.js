// String enums
export const Emulator = {
  Famicom: 'Famicom',
  SuperFamicom: 'Super Famicom',
  MegaDrive: 'Mega Drive'
}

export const EmulatorEvent = {
  FrameStart: 'frame.start',
  FrameEnd: 'frame.end',
  Resize: 'resize',
}

export const Settings = {
  ['Super Famicom']: {
    CPU: {
      Lockstep: 'cpu/lockstep',
      Fastmath: 'cpu/fastmath',
      Overclock: 'cpu/overclock'
    },
    SMP: {
      Lockstep: 'smp/lockstep'
    },
    DSP: {
      Enabled: 'dsp/enabled'
    }, 
    PPU: {
      Skipframe: 'ppu/skipframe'
    }
  },
  ['Famicom']: {
    PPU: {
      Skipframe: 'ppu/skipframe',
      Overscan: 'ppu/overscan'
    },
    CPU: {
      SyncOnce: 'cpu/synconce'
    }
  }
}

let lib;
let initialized = false

import Module from './byuu-web-lib.js'

// The following are injected at build time
export const version = '$version'
export const commit = '$commit'
export const dirty = $dirty

function getBinaryPath() {
  try {
    return require('./byuu-web-lib.wasm').default
  } catch (error) {
    throw new Error('Failed to load WASM code - see https://github.com/Wizcorp/byuu-web#failed-to-load-wasm-code')
  }
}

function getModule() {
  if (!initialized) {
    throw new Error('Emulator module was not initialized before use')
  }

  return lib
}

import EventEmitter from 'eventemitter3'
const byuu = new EventEmitter()

// Create container
const container = document.createElement('div');
Object.assign(container.style, {
  position: 'relative',
  display: 'flex',
  height: '100%',
  width: '100%',
  flexDirection: 'column',
  flexWrap: 'wrap',
  alignItems: 'flex-start'
});

// Create canvas
const canvas = document.createElement('canvas')
const getContext = canvas.getContext.bind(canvas);

canvas.id = 'canvas';
Object.assign(canvas.style, {
  height: '100%',
  width: '100%',
  objectFit: 'contain',
});

const contextOptions = {
  // Required to enable screenshots
  preserveDrawingBuffer: true,
  // Optimize for performance
  powerPreference: 'high-performance',
  alpha: false,
  desynchronized: true,
  antialias: false,
};

canvas.getContext = function(contextId, options) {
  options = options || {};

  if (contextId === 'webgl') {
    Object.assign(options, contextOptions);
  }
  
  return getContext(contextId, options);
};

container.appendChild(canvas)

byuu.displayRatio = 1;

byuu.initialize = async function (parent, ctxOptions) {
  if (!parent) {
    throw new Error('container parameter is not defined')
  }

  ctxOptions = ctxOptions || {};
  Object.assign(contextOptions, ctxOptions);

  parent.appendChild(container)

  // Emscripten's SDL port enforces that the id of the canvas be canvas
  // https://github.com/emscripten-ports/SDL2/blob/952f889879ba3d934249fc7d93b992f91f75a5cd/src/video/emscripten/SDL_emscriptenvideo.c#L217
  const domElementWithIDCanvas = document.getElementById('canvas')
  if (domElementWithIDCanvas && !canvas.isSameNode(domElementWithIDCanvas)) {
    throw new Error('The DOM ID attribute "canvas" is reserved by byuu for it\'s own canvas')
  }

  return new Promise((resolve) => {
    // Module isn't a real promise, and unless we set
    // things as follow the code seem to tight-loop
    Module({
      locateFile: (filename, prefix) =>  {
        if (filename === 'byuu-web-lib.wasm') {
          return getBinaryPath()
        } 

        return prefix + filename
      },
      canvas
    }).then((result) => {
      lib = result
      lib.initialize(document.title || 'byuu')
  
      // Set callbacks, patch into event emission
      lib.onFrameStart(() => byuu.emit('frame.start'))
      lib.onFrameEnd(() => byuu.emit('frame.end'))
      lib.onResize((width, height) => {
        byuu.displayRatio = width / height;
        byuu.emit('resize', { width, height });
      })
      
      initialized = true
      resolve()
    })
  })
}

byuu.terminate = () => {
  byuu.stop()
  byuu.unload()
  getModule().terminate()
  container.parentElement.removeChild(container)
}

byuu.setFit = (fit) => Object.assign(canvas.style, { objectFit: fit }) 

byuu.setPosition = (position) => Object.assign(canvas.style, { objectPosition: position })

byuu.getCanvas = () => canvas

byuu.getEmulatorForFilename = (filename) => getModule().getEmulatorForFilename(filename)

byuu.setEmulator = (emulator) => getModule().setEmulator(emulator)

byuu.setEmulatorForFilename = (filename) => getModule().setEmulatorForFilename(filename)

byuu.load = (romData, saveFiles) => getModule().load(romData, saveFiles || {})

byuu.loadURL = async (url, saveFiles) => new Promise((resolve, reject) => {
  getModule().loadURL(url, saveFiles || {}, (errorString, info) => {
    if (errorString) {
      return reject(new Error(errorString))
    }

    resolve(info)
  })
})

byuu.unload = () => getModule().unload()

byuu.start = () => getModule().start()

byuu.run = () => getModule().run()

byuu.stop = () => getModule().stop()

byuu.isStarted = () => getModule().isStarted()

byuu.isRunning = () => getModule().isRunning()

byuu.setVolume = (volume) => getModule().setVolume(volume)

byuu.setMute = (mute) => getModule().setMute(mute)

byuu.connectPeripheral = (portName, peripheralName) => getModule().connectPeripheral(portName, peripheralName)

byuu.disconnectPeripheral = (portName) => getModule().disconnectPeripheral(portName)

byuu.setButton = (portName, buttonName, value) => getModule().setButton(portName, buttonName, value)

byuu.getROMInfo = (filename, romData) => getModule().getROMInfo(filename, romData)

byuu.stateSave = async () => new Promise((resolve) => {
  getModule().stateSave(({ buffer, byteOffset, byteLength }) => {
    const array = new Uint8Array(buffer.slice(byteOffset, byteOffset + byteLength))
    resolve(array)
  })
})

byuu.stateLoad = (stateData) => getModule().stateLoad(stateData)

// This mangling is required to avoid issues with IndexedDB storage
// (e.g some API seem to access the buffer attribute of views, which in this case
// is the entire heap!)
byuu.save = () => {
  const saveFiles = getModule().save()
  for (const [filename, { buffer, byteOffset, byteLength }] of Object.entries(saveFiles)) {
    saveFiles[filename] = new Uint8Array(buffer.slice(byteOffset, byteOffset + byteLength))
  }

  return saveFiles
}

export default byuu
