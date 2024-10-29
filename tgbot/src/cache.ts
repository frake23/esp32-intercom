import Redis from 'ioredis';

const redis = new Redis(process.env.REDIS_PATH);

export class CacheClient {
    static set(key: string, value: string | number | Buffer, seconds?: number) {
        if (seconds) {
            return redis.set(key, value, 'EX', seconds);
        }

        return redis.set(key, value);
    }

    static get(key: string) {
        return redis.get(key);
    }

    static del(key: string) {
        return redis.del(key);
    }
}
