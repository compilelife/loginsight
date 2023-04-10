class Dispatcher {
  callbacks = new Map<string, Function>();

  registerEx(instance:string, func: string, callback: Function) {
    this.register(`${instance}-${func}`, callback);
  }

  unregisterEx(instance:string, func: string) {
    this.unregister(`${instance}-${func}`)
  }

  register(name: string, callback: Function) {
    this.callbacks.set(name, callback);
  }

  unregister(name: string) {
    this.callbacks.delete(name);
  }

  unregisterAll(who:string) {
    const prefix=`${who}-`
    this.callbacks.forEach((_,key)=>{
      if (key.startsWith(prefix))
        this.callbacks.delete(key)
    })
  }

  dispatch(...args: any[]) {
    const who = args[0]
    const target = this.callbacks.get(who)
    console.log('dispatch', who, target ? '' : 'no handler')
    if (target) {
      args.shift()
      return target(...args)
    }
  }
}

export const dispatcher = new Dispatcher();