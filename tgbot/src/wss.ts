import net from 'node:net';
import { bot } from './bot';
import { flatsRepo } from './flats';
import { Markup } from 'telegraf';

export let clientSocket: net.Socket | null = null;

// Create a server instance
export const server = net.createServer();

let currentSocket: net.Socket | null = null;
let currentCommand: 'start' | 'photo' | 'cancel' | 'finish' | null = null;
let currentFlat: number | null = null;

const photoController = async () => {
    // Variables to keep track of the incoming data
    let imageSize: number | null = null; // The size of the image to receive
    let imageBuffer: Buffer | null = null; // Buffer to store the image data
    let receivedBytes = 0; // Number of bytes received so far
    let headerBuffer = Buffer.alloc(4); // Buffer to accumulate the image size header
    let headerBytesReceived = 0; // Number of header bytes received

    return async (data: Buffer) => {
        let offset = 0; // Offset in the data buffer

        // Process the received data
        while (offset < data.length) {
            if (imageSize === null) {
                // We are still reading the image size (header)

                // Calculate how many bytes we need to complete the header
                const bytesNeeded = 4 - headerBytesReceived;
                const bytesAvailable = data.length - offset;
                const bytesToCopy = Math.min(bytesNeeded, bytesAvailable);

                // Copy bytes from the received data into the header buffer
                data.copy(
                    headerBuffer,
                    headerBytesReceived,
                    offset,
                    offset + bytesToCopy
                );
                headerBytesReceived += bytesToCopy;
                offset += bytesToCopy;

                // Check if we have received the full header
                if (headerBytesReceived === 4) {
                    // Read the image size from the header buffer (big-endian order)
                    imageSize = headerBuffer.readUInt32BE(0);
                    console.log(`Image size to receive: ${imageSize} bytes`);

                    // Allocate a buffer to store the image data
                    imageBuffer = Buffer.alloc(imageSize);
                    receivedBytes = 0;
                }
            } else {
                // We have the image size; now receive the image data

                // Calculate how many bytes we need to complete the image
                const bytesNeeded = imageSize - receivedBytes;
                const bytesAvailable = data.length - offset;
                const bytesToCopy = Math.min(bytesNeeded, bytesAvailable);

                // Copy bytes from the received data into the image buffer
                data.copy(
                    imageBuffer!,
                    receivedBytes,
                    offset,
                    offset + bytesToCopy
                );
                receivedBytes += bytesToCopy;
                offset += bytesToCopy;

                // Check if we have received the full image
                if (receivedBytes === imageSize) {
                    console.log('Image received completely');
                    const flat = (await flatsRepo.getManyByNumber(123))[0];
                    bot.telegram.sendPhoto(flat.chatId, {
                        source: imageBuffer!,
                    });

                    // Reset variables to receive the next image
                    imageSize = null;
                    imageBuffer = null;
                    receivedBytes = 0;
                    headerBytesReceived = 0;
                    currentCommand = null;
                }
            }
        }
    };
};

bot.action('photo', (ctx) => {
    if (!currentSocket || !ctx.flat || ctx.flat.number !== currentFlat) {
        return ctx.sendMessage('Сессия сейчас неактивна');
    }
    currentSocket.write('photo');
});

bot.action('accept', (ctx) => {
    if (!currentSocket || !ctx.flat || ctx.flat.number !== currentFlat) {
        return ctx.sendMessage('Сессия сейчас неактивна');
    }
    currentSocket.write('accept');
    return ctx.editMessageText('✅ Пускаем');
});

bot.action('reject', (ctx) => {
    if (!currentSocket || !ctx.flat || ctx.flat.number !== currentFlat) {
        return ctx.sendMessage('Сессия сейчас неактивна');
    }
    currentSocket.write('reject');
    return ctx.editMessageText('❌ Не пускаем');
});

const startController = async (data: Buffer) => {
    currentFlat = Number(data.toString().trim());
    const flats = await flatsRepo.getManyByNumber(currentFlat);
    if (flats.length === 0) {
        currentSocket?.write('not_found');
        currentSocket?.end();
        currentSocket = null;
    } else {
        const promises = flats.map((flat) =>
            bot.telegram.sendMessage(
                flat.chatId,
                'Кто-то хочет зайти!',
                Markup.inlineKeyboard([
                    Markup.button.callback('📸 Фото', 'photo'),
                    Markup.button.callback('✅ Пустить', 'accept'),
                    Markup.button.callback('❌ Не пускать', 'reject'),
                ])
            )
        );
        await Promise.all(promises);
    }

    currentCommand = null;
};

const espCommandsMapping = {
    photo: photoController(),
    start: startController,
};

server.on('connection', (socket) => {
    console.log('Client connected');

    currentCommand = 'start';

    // Handle incoming data from the client
    socket.on('data', async (data) => {
        if (currentCommand) {
            await espCommandsMapping[currentCommand](data);
        } else {
            currentCommand = data.toString().trim() as any;
        }
    });

    // Handle client disconnection
    socket.on('end', () => {
        console.log('Client disconnected');
        currentSocket = null;
    });

    // Handle socket errors
    socket.on('error', (err) => {
        console.error('Socket error:', err);
    });
});

// Handle server errors
server.on('error', (err) => {
    console.error('Server error:', err);
});
