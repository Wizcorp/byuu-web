import EventEmitter from 'eventemitter3'

/**
 * Supported emulation cores
 * 
 * @todo add all other emulation cores
 */
export const enum Emulator {
  Famicom = 'Famicom',
  SuperFamicom = 'Superfamicom',
}

/**
 * Events that are fired by byuu
 */
export const enum Event {
  FrameStart = 'frame.start',
  FrameEnd = 'frame.end',
}

/**
 * ROM metadata node 
 */
export interface BMLNode {
  name: string
  value: string
  children: BMLNode[]
}

/**
 * ROM related information
 * 
 * The information provided is for both the console emulator and the
 * data found in the ROM itself. Note that data attributes may vary depending
 * on emulated console, cartridge format, and cartridges.
 */
export interface ROMInfo {
  emulator: {
    name: Emulator
    ports: string[]
    buttons: string[]
  }
  rom: BMLNode[]
}

/**
 * List of files generated on save
 * 
 * Each file represent the data of a RAM component (generally on cartridge).
 */
export interface SaveFiles {
  [filename: string]: Uint8Array
}

/**
 * byuu-web
 * 
 * byuu-web is a port of https://github.com/byuu/byuu available both
 * as a standalone web application and an embeddable library.
 * 
 * This library will require the DOM to already have a <canvas> element
 * width its id set to "canvas".
 */
declare class Byuu extends EventEmitter<Event> {
  /**
   * Initialize the module
   * 
   * This method needs to be called before any other methods can be used
   * 
   * @param container The DOM element to which the canvas element byuu renders in will be appended
   * @param width The width to apply to the render canvas
   * @param height The height to apply to the render canvas
   */
  public initialize(container: HTMLElement, width: number, height: number) : Promise<void>

  /**
   * Stop, unload byuu, and remove canvas
   */
  public terminate() : boolean

  /**
   * Find applicable emulator for a given file name
   * 
   * Based on the extension, it is possible to infer the desired emulator.
   * Will return an empty string if no candidate emulators are found.
   * 
   * @param filename ROM file name with extension
   */
  public getEmulatorForFilename(filename: string) : Emulator | ""

  /**
   * Explicitly set the emulator by name
   * 
   * @param emulator 
   */
  public setEmulator(emulator: Emulator) : boolean

  /**
   * Set the emulator based on a given file name
   * 
   * @param filename ROM file name with extension
   */
  public setEmulatorForFilename(filename: string) : boolean

  /**
   * Load a ROM
   * 
   * @param romData Byte array with the ROM's content
   * @param saveFiles List of files generated on save (see Emulator.save)
   */
  public load(romData: Uint8Array, saveFiles?: SaveFiles) : ROMInfo

  /**
   * Download and load a remote ROM
   * 
   * @param url Remote URL of the ROM
   * @param saveFiles List of files generated on save (see Emulator.save)
   */
  public loadURL(url: string, saveFiles?: SaveFiles) : Promise<ROMInfo>

  /**
   * Unload the current game, then unload the current emulator
   */
  public unload() : void

  /**
   * Start the emulator's render loop
   */
  public start() : boolean

  /**
   * Run one render cycle
   * 
   * This can be useful while debugging, or for implementing custom rendering loops;
   * otherwise it is generally not needed.
   */
  public run() : void

  /**
   * Stop the emulator's render loop
   */
  public stop() : boolean

  /**
   * Check if the emulator is started
   */
  public isStarted() : boolean
  
  /**
   * Check if the emulator is running a render cycle
   * 
   * Unlike Emulator.start, this checks if a cycle is ongoing regardless
   * of whether Emulator.start has been called (e.g. if a run cycle was manually triggered).
   */
  public isRunning() : boolean

  /**
   * Resize the render canvas
   * 
   * @param width The width to apply to the render canvas
   * @param height The height to apply to the render canvas
   */
  public resize(width: number, height: number) : void

  /**
   * Connect a new peripheral
   * 
   * Note that the port and peripheral names are part of the ROMInfo returned
   * when calling either Emulator.load, Emulator.loadURL, or Emulator.getROMInfo.
   *  
   * @param portName The physical port slot (example: Controller Port 1)
   * @param peripheralName The peripheral type (example: Gamepad)
   */
  public connectPeripheral(portName: string, peripheralType: string) : boolean

  /**
   * Disonnect a peripheral connected on a given port
   * 
   * Note that the port and peripheral names are part of the ROMInfo returned
   * when calling either Emulator.load, Emulator.loadURL, or Emulator.getROMInfo.
   *  
   * @param portName The physical port slot (example: Controller Port 1)
   */
  public disconnectPeripheral(portName: string) : boolean

  /**
   * Set the button state for a controller on a given port
   * 
   * Note that the port and button names are part of the ROMInfo returned
   * when calling either Emulator.load, Emulator.loadURL, or Emulator.getROMInfo.
   *  
   * @param portName The physical port slot (example: Controller Port 1)
   * @param buttonName The name of the button (see ROMInfo for details)
   * @param value The value (0/1 for buttons, value between -32768 and +32767 for axises)
   */
  public setButton(portName: string, buttonName: string, value: number) : boolean; 

  /**
   * Get information regarding a given ROM
   * 
   * Information includes general console information and cartridge data. Note that the data
   * may vary based on consoles, cartridges, and cartridges type.
   * 
   * @param filename File name
   * @param romData Byte array with the ROM's content
   */
  public getROMInfo(filename: string, romData: Uint8Array) : ROMInfo | null

  /**
   * Take a snapshot of the current game's state which can be restored later
   * 
   * This differs from Emulator.save by taking a snapshot at a specific point in time. Note
   * that the size of a state will be up to 10x bigger than a normal save depending on the console.
   */
  public stateSave() : Promise<Uint8Array>

  /**
   * Load a state snapshot
   * 
   * @param stateData State data from Emulator.stateSave
   */
  public stateLoad(stateData: Uint8Array) : boolean

  /**
   * Retrieve the game save files
   * 
   * This will return a map of files which can be provided back when loading a game during a future
   * session. Each file represent a RAM snapshot (which would generally be on the cartridge).
   */
  public save() : SaveFiles
}

declare const byuu: Byuu

declare namespace byuu {}

export default byuu