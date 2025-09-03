# Use Node.js base image
FROM node:18

# Set working directory inside container
WORKDIR /usr/src/app

# Copy package.json (dependencies)
COPY package*.json ./

# Install dependencies
RUN npm install

# Copy app source
COPY . .

# Expose port
EXPOSE 3000

# Start the app
CMD ["node", "app/server.js"]
