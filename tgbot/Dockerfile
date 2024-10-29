# Use the official Node.js v22 image from the Docker Hub
FROM node:22

# Create and change to the app directory
WORKDIR /usr/src/app

# Copy package.json and package-lock.json
COPY package*.json ./

# Install the dependencies
RUN npm install

# Copy the rest of the application code
COPY . .

# Expose the port your application runs on
EXPOSE 3001

# Start the Node.js application
CMD ["npm", "run", "dev"]