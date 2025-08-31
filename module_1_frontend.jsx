import React, { useEffect, useState } from 'react';
import { useNavigate } from 'react-router-dom';

const Groot = () => {
  const [score, setScore] = useState(0);
  const [level, setLevel] = useState(1);
  const [achievements, setAchievements] = useState([]);
  const navigate = useNavigate();
  const userId = localStorage.getItem('userId');

  // Fetch progress on mount
  useEffect(() => {
    fetch(`http://localhost:5000/groot/${userId}`)
      .then(res => res.json())
      .then(data => {
        if (data) {
          setScore(data.score || 0);
          setLevel(data.level || 1);
          setAchievements(data.achievements || []);
        }
      });
  }, [userId]);

  const saveProgress = async () => {
    await fetch(`http://localhost:5000/groot/update/${userId}`, {
      method: 'PUT',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ score, level, achievements })
    });
    alert('🪵 Groot progress saved!');
  };

  return (
    <div>
      <h1>🌱 Project I-Am-Groot</h1>
      <p>Score: {score}</p>
      <p>Level: {level}</p>
      <p>Achievements: {achievements.join(', ')}</p>

      <button onClick={() => {
        setScore(score + 500);
        setLevel(level + 1);
        setAchievements([...achievements, `Milestone ${level + 1}`]);
      }}>
        Complete Challenge
      </button>

      <button onClick={saveProgress}>💾 Save Progress</button>
      <button onClick={() => navigate('/stark')}>Next Module ➡️</button>
    </div>
  );
};

export default Groot;
