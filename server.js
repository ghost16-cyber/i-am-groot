const express = require("express");
const { Pool } = require("pg");

const app = express();
const PORT = 3000;

// DB connection (read from docker-compose env)
const pool = new Pool({
  host: process.env.DB_HOST || "localhost",
  user: process.env.DB_USER || "postgres",
  password: process.env.DB_PASSWORD || "postgres",
  database: process.env.DB_NAME || "personel_db",
  port: process.env.DB_PORT || 5432,
});

app.get("/", (req, res) => {
  res.send("🚀 Hello from Dockerized Node.js + Postgres App!");
});

// Example endpoint: query database
app.get("/users", async (req, res) => {
  try {
    const result = await pool.query("SELECT NOW() as server_time");
    res.json(result.rows[0]);
  } catch (err) {
    console.error(err);
    res.status(500).send("Database error");
  }
});

app.listen(PORT, () => {
  console.log(`Server running at http://localhost:${PORT}`);
});
