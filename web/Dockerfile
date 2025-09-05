# Use official Node.js image (small variant)
FROM node:18-slim

# Set working directory inside container
WORKDIR /usr/src/app

# Copy package files first (for caching npm install)
COPY package*.json ./

# Install dependencies
RUN npm install

# Copy rest of the app code
COPY . .

# Expose port 3000
EXPOSE 3000

# Start the app
CMD ["npm", "start"]
