import React from "react";
import { useLocation, Outlet, NavLink } from "react-router-dom";
import Sidebar from "./Sidebar";
import Header from "./Header";

function Main() {

  return (
    <>
        {/* ======= Header ======= */}
        <header id="header" className="header fixed-top d-flex align-items-center">
            <Header/>
        </header>{/* End Header */}


        {/* ======= Sidebar ======= */}
        <aside id="sidebar" className="sidebar">
            <Sidebar/>
        </aside>{/* End Sidebar */}


        {/* ======= Main ======= */}
        <main id="main" className="main">
            <Outlet/>
        </main>{/* End Main */}


        {/* ======= Footer ======= */}
        <footer id="footer" className="footer">
            <div className="copyright">
                &copy; Copyright <strong><span>NiceAdmin</span></strong>. All Rights Reserved
            </div>
            <div className="credits">
                {/* All the links in the footer should remain intact. */}
                {/* You can delete the links only if you purchased the pro version. */}
                {/* Licensing information: https://bootstrapmade.com/license/ */}
                {/* Purchase the pro version with working PHP/AJAX contact form: https://bootstrapmade.com/nice-admin-bootstrap-admin-html-template/ */}
                Designed by <a href="https://bootstrapmade.com/">BootstrapMade</a>
            </div>
        </footer>{/* End Footer */}

        <a href="#" className="back-to-top d-flex align-items-center justify-content-center"><i className="bi bi-arrow-up-short"></i></a>

        {/* Vendor JS Files */}
        {/*<script src="../src/assets/vendor/apexcharts/apexcharts.min.js"></script>*/}
        {/*<script src="../src/assets/vendor/bootstrap/js/bootstrap.bundle.min.js"></script>*/}
        {/*<script src="../src/assets/vendor/chart.js/chart.umd.js"></script>*/}
        {/*<script src="../src/assets/vendor/echarts/echarts.min.js"></script>*/}
        {/*<script src="../src/assets/vendor/quill/quill.min.js"></script>*/}
        {/*<script src="../src/assets/vendor/simple-datatables/simple-datatables.js"></script>*/}
        {/*<script src="../src/assets/vendor/tinymce/tinymce.min.js"></script>*/}
        {/*<script src="../src/assets/vendor/php-email-form/validate.js"></script>*/}

        {/* Template Main JS File */}
        {/*<script src="../src/assets/js/main.js"></script>*/}
    </>
  );
}

export default Main;
