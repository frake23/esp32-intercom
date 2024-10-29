import net from 'node:net';

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
        console.log('Received from client:', data);

        if (text === 'PING') {
            console.log('Received PING, sending PONG');
            socket.write('PONG\n');
        } else if (text === 'PONG') {
            console.log('Received PONG');
            // Implement your logic if needed
        } else {
            console.log('Received:', data);
            // Echo the data back to the client
            socket.write('Echo: ${data}\n');
        }
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
