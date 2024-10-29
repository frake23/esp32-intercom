import { bot } from './bot';
import { server } from './wss';
import mongoose from 'mongoose';

await mongoose.connect(process.env.MONGO_URI as string, {
    user: process.env.MONGO_USER,
    pass: process.env.MONGO_PASSWORD,
});

bot.launch(() => {
    console.log('BOT started');
});

server.listen(Number(process.env.WSS_PORT), () => {
    console.log('Socket Server started');
});
