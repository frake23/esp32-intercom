declare global {
    namespace NodeJS {
        interface ProcessEnv {
            BOT_TOKEN: string;
            REDIS_PATH: string;
            WSS_PORT: string;
            MONGO_URI: string;
            MONGO_PASSWORD: string;
            MONGO_USER: string;
        }
    }
}

export {};
