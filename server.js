const express = require('express');
const mongoose = require('mongoose');
const cors = require('cors');

const authRoutes = require('./routes/auth');
const grootRoutes = require('./routes/groot');

const app = express();
app.use(cors());
app.use(express.json());

mongoose.connect('mongodb://localhost:27017/marvelDB')
  .then(() => console.log('✅ MongoDB Connected'))
  .catch(err => console.error('❌ MongoDB Error', err));

app.use('/auth', authRoutes);
app.use('/groot', grootRoutes);  

app.listen(5000, () => console.log('🚀 Server running on http://localhost:5000'));
