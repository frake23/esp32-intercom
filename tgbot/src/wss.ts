import net from 'node:net';

export let clientSocket: net.Socket | null = null;

// Create a server instance
export const server = net.createServer((socket) => {
    console.log(
        'Client connected:',
        socket.remoteAddress + ':' + socket.remotePort
    );

    // Set encoding for incoming data
    socket.setEncoding('utf8');

    // Handle incoming data
    socket.on('data', (data) => {
        const text = data.toString().trim();
        console.log(text);
    });

    // Handle client disconnect
    socket.on('end', () => {
        console.log(
            'Client disconnected:',
            socket.remoteAddress + ':' + socket.remotePort
        );
    });

    // Handle errors
    socket.on('error', (err) => {
        console.error('Socket error:', err);
    });
});

// Handle server errors
server.on('error', (err) => {
    console.error('Server error:', err);
});
