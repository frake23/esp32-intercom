declare global {
  namespace NodeJS {
    interface ProcessEnv {
      BOT_TOKEN: string;
      REDIS_PATH: string;
      WSS_PORT: string;
    }
  }
}

export {};
