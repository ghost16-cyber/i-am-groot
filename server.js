const express = require("express");
const app = express();

app.get("/", (req, res) => {
  res.send("🚀 Hello from your Mini Game Server!");
});

app.listen(5000, () => {
  console.log("✅ Server running on http://localhost:5000");
});
