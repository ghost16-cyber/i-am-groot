const express = require('express');
const router = express.Router();
const User = require('../models/User');

// ✅ Update Groot progress
router.put('/update/:userId', async (req, res) => {
  const { level, score, achievements } = req.body;

  try {
    const updatedUser = await User.findByIdAndUpdate(
      req.params.userId,
      {
        $set: {
          'progress.groot.level': level,
          'progress.groot.score': score,
          'progress.groot.achievements': achievements
        }
      },
      { new: true }
    );

    if (!updatedUser) return res.status(404).json({ error: 'User not found' });
    res.json({ message: 'Progress updated', groot: updatedUser.progress.groot });

  } catch (err) {
    res.status(500).json({ error: 'Server error' });
  }
});

// ✅ Get Groot progress
router.get('/:userId', async (req, res) => {
  try {
    const user = await User.findById(req.params.userId);
    if (!user) return res.status(404).json({ error: 'User not found' });

    res.json(user.progress.groot);
  } catch (err) {
    res.status(500).json({ error: 'Server error' });
  }
});

module.exports = router;
