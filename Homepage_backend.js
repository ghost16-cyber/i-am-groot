const express = require('express');
const bcrypt = require('bcrypt');
const jwt = require('jsonwebtoken');
const User = require('../models/User');
const router = express.Router();

router.post('/signup', async (req, res) => {
  const { username, password } = req.body;
  const hash = await bcrypt.hash(password, 10);
  const user = new User({ username, password: hash });
  await user.save();
  res.status(201).json({ message: 'User created' });
});

router.get('/progress/:userId', async (req, res) => {
  const user = await User.findById(req.params.userId);
  res.json({ grootScore: user.progress.groot.score });  // ✅ Example usage
});

router.post('/login', async (req, res) => {
  const { username, password } = req.body;
  const user = await User.findOne({ username });
  if (!user || !(await bcrypt.compare(password, user.password)))
    return res.status(401).json({ error: 'Invalid credentials' });
  
  const token = jwt.sign({ id: user._id }, 'secret');
  res.json({ token });
});

module.exports = router;

if (data.token) {
  localStorage.setItem('token', data.token);

  // ✅ Decode token to extract user ID
  const payload = JSON.parse(atob(data.token.split('.')[1]));
  localStorage.setItem('userId', payload.id);

  navigate('/groot');
}s
