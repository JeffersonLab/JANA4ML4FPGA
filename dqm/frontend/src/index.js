import React from "react";
import ReactDOM from "react-dom/client";
import { RouterProvider, createBrowserRouter, } from "react-router-dom";
import {SWRConfig, useSWRConfig} from 'swr'
import MainLayout from "layouts/Main.jsx";
import VolatilityPage from "./pages/VolatilityPage";

// CSS imports
import "../src/assets/vendor/bootstrap/css/bootstrap.min.css"
import "../src/assets/vendor/bootstrap-icons/bootstrap-icons.css"
import "../src/assets/vendor/boxicons/css/boxicons.min.css"
import "../src/assets/vendor/quill/quill.snow.css"
import "../src/assets/vendor/quill/quill.bubble.css"
import "../src/assets/vendor/remixicon/remixicon.css"
import "../src/assets/vendor/simple-datatables/style.css"
import "../src/assets/css/style.css"


// Routes
const router = createBrowserRouter([
  {
    path: "/",
    element: <MainLayout />,
    children: [
        {
            path: "volatility",
            element: <VolatilityPage />,
        }
    ]
  },
]);

console.log(router)


ReactDOM.createRoot(document.getElementById("root")).render(
  <React.StrictMode>
    <SWRConfig
        value={{
          refreshInterval: 3000,
          fetcher: (resource, init) => fetch(resource, init).then(res => res.json())
      }}>
      <RouterProvider router={router} />
    </SWRConfig>
  </React.StrictMode>
);