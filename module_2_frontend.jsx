import React, { useState, useEffect } from 'react';
import { useNavigate } from 'react-router-dom';

const StarkIndustries = () => {
  const [dashboardConfig, setDashboardConfig] = useState({});
  const [alerts, setAlerts] = useState([]);
  const [reports, setReports] = useState([]);
  const userId = localStorage.getItem('userId');
  const navigate = useNavigate();

  useEffect(() => {
    fetch(`http://localhost:5000/stark/${userId}`)
      .then(res => res.json())
      .then(data => {
        if (data) {
          setDashboardConfig(data.dashboardConfig || {});
          setAlerts(data.alerts || []);
          setReports(data.reports || []);
        }
      });
  }, [userId]);

  const saveProgress = async () => {
    await fetch(`http://localhost:5000/stark/update/${userId}`, {
      method: 'PUT',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ dashboardConfig, alerts, reports })
    });
    alert('🧠 Stark progress saved!');
  };

  return (
    <div style={{ padding: '2rem' }}>
      <h1>🧠 Project Starkledger</h1>
      <p>Manage Stark Industries dashboard data</p>
      <button onClick={saveProgress}>💾 Save Progress</button>
      <button onClick={() => navigate('/spiderman')}>Next Module ➡️</button>
    </div>
  );
};

export default StarkIndustries;
