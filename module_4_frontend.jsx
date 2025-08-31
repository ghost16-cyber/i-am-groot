import React, { useState, useEffect } from 'react';

const DrStrange = () => {
  const [spellbooks, setSpellbooks] = useState([]);
  const [searchHistory, setSearchHistory] = useState([]);
  const userId = localStorage.getItem('userId');

  useEffect(() => {
    fetch(`http://localhost:5000/drstrange/${userId}`)
      .then(res => res.json())
      .then(data => {
        if (data) {
          setSpellbooks(data.spellbooks || []);
          setSearchHistory(data.searchHistory || []);
        }
      });
  }, [userId]);

  const saveProgress = async () => {
    await fetch(`http://localhost:5000/drstrange/update/${userId}`, {
      method: 'PUT',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ spellbooks, searchHistory })
    });
    alert('🔮 Dr Strange progress saved!');
  };

  return (
    <div style={{ padding: '2rem' }}>
      <h1>🔮 Project Mystic</h1>
      <button onClick={() => setSpellbooks([...spellbooks, { title: "New Spell", power: "🔥" }])}>
        ➕ Add Spellbook
      </button>
      <button onClick={saveProgress}>💾 Save Progress</button>
    </div>
  );
};

export default DrStrange;
