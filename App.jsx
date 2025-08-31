import { BrowserRouter as Router, Routes, Route } from 'react-router-dom';
import Homepage from './pages/Homepage';
import Groot from './modules/Groot/Groot';
import StarkIndustries from './modules/StarkIndustries/StarkIndustries';
import Spiderman from './modules/Spiderman/Spiderman';
import DrStrange from './modules/DrStrange/DrStrange';

function App() {
  return (
    <Router>
      <Routes>
        <Route path="/" element={<Homepage />} />
        <Route path="/groot" element={<Groot />} />
        <Route path="/stark" element={<StarkIndustries />} />
        <Route path="/spiderman" element={<Spiderman />} />
        <Route path="/drstrange" element={<DrStrange />} />
      </Routes>
    </Router>
  );
}

export default App;
