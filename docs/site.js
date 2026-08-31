(function () {
  "use strict";

  var menuButton = document.querySelector(".menu-toggle");
  var nav = document.querySelector(".site-nav");
  var progress = document.querySelector(".reading-progress");
  var spoilerButtons = document.querySelectorAll("[data-spoiler-toggle]");
  var spoilerContent = document.querySelector("#route-content");

  if (menuButton && nav) {
    menuButton.addEventListener("click", function () {
      var open = nav.classList.toggle("is-open");
      menuButton.setAttribute("aria-expanded", String(open));
      menuButton.textContent = open ? "CLOSE" : "MENU";
    });

    nav.addEventListener("click", function (event) {
      if (event.target.tagName === "A") {
        nav.classList.remove("is-open");
        menuButton.setAttribute("aria-expanded", "false");
        menuButton.textContent = "MENU";
      }
    });
  }

  if (spoilerButtons.length && spoilerContent) {
    spoilerButtons.forEach(function (spoilerButton) {
      spoilerButton.addEventListener("click", function () {
      var hidden = spoilerContent.hasAttribute("hidden");
      if (hidden) {
        spoilerContent.removeAttribute("hidden");
      } else {
        spoilerContent.setAttribute("hidden", "");
      }
      spoilerButtons.forEach(function (button) {
        button.textContent = hidden ? "HIDE SPOILER ROUTE" : "SHOW SPOILER ROUTE";
        button.setAttribute("aria-expanded", String(hidden));
      });
      });
    });
  }

  function updateProgress() {
    if (!progress) return;
    var scrollable = document.documentElement.scrollHeight - window.innerHeight;
    var percent = scrollable > 0 ? (window.scrollY / scrollable) * 100 : 0;
    progress.style.width = Math.min(100, Math.max(0, percent)) + "%";
  }

  window.addEventListener("scroll", updateProgress, { passive: true });
  window.addEventListener("resize", updateProgress);
  updateProgress();

  var sections = document.querySelectorAll("main section[id]");
  var links = document.querySelectorAll('.site-nav a[href^="#"]');
  if ("IntersectionObserver" in window && sections.length && links.length) {
    var observer = new IntersectionObserver(function (entries) {
      entries.forEach(function (entry) {
        if (!entry.isIntersecting) return;
        links.forEach(function (link) {
          link.classList.toggle("is-active", link.getAttribute("href") === "#" + entry.target.id);
        });
      });
    }, { rootMargin: "-28% 0px -60% 0px" });
    sections.forEach(function (section) { observer.observe(section); });
  }
}());
