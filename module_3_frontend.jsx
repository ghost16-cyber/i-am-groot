import React, { useState, useEffect } from 'react';
import { useNavigate } from 'react-router-dom';

const Spiderman = () => {
  const [missions, setMissions] = useState([]);
  const [calendarPrefs, setCalendarPrefs] = useState({});
  const userId = localStorage.getItem('userId');
  const navigate = useNavigate();

  useEffect(() => {
    fetch(`http://localhost:5000/spiderman/${userId}`)
      .then(res => res.json())
      .then(data => {
        if (data) {
          setMissions(data.missions || []);
          setCalendarPrefs(data.calendarPrefs || {});
        }
      });
  }, [userId]);

  const saveProgress = async () => {
    await fetch(`http://localhost:5000/spiderman/update/${userId}`, {
      method: 'PUT',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ missions, calendarPrefs })
    });
    alert('🕷️ Spiderman progress saved!');
  };

  return (
    <div style={{ padding: '2rem' }}>
      <h1>🕷️ SpideySense Scheduler</h1>
      <button onClick={() => setMissions([...missions, { name: "New Mission", date: new Date() }])}>
        ➕ Add Mission
      </button>
      <button onClick={saveProgress}>💾 Save Progress</button>
      <button onClick={() => navigate('/drstrange')}>Next Module ➡️</button>
    </div>
  );
};

export default Spiderman;
