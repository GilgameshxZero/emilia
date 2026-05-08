import {
	prependRelativePaths,
	registerComponent
} from "../../scripts/component.js";
import "./icon.js";

registerComponent(
	`snapshot`,
	`erlija-past`,
	class extends HTMLElement {
		onDomLoad() {
			// Snapshots are fake subcomponentLoaded immediately, but again when the snapshot path is set. The first subcomponentLoad is for the two icons.
			const icons = [...this.shadowRoot.querySelectorAll(`emilia-icon`)];
			this.subcomponentLoad = new Promise((resolve) => {
				Promise.all([
					this.resourceLoad,
					...icons.map((icon) => {
						return icon.resourceLoad;
					})
				]).then(() => {
					Promise.all(
						icons.map((icon) => {
							return icon.subcomponentLoad;
						})
					).then(() => {
						resolve();
					});
				});
			});

			// Set click handlers for both icons to go back to timeline.
			icons.forEach((icon) => {
				icon.addEventListener(`click`, () => {
					this.parentNode.host.toTimeline();
				});
			});
		}

		// Actually load the snapshot text, and reset subcomponentLoad. resourceLoad is completed at this point.
		setSnapshotName(name) {
			let htmlPath;
			this.subcomponentLoad = new Promise((resolve) => {
				fetch(`/api/snapshots/${name}.json`)
					.then((res) => {
						return res.json();
					})
					.then((json) => {
						htmlPath = json.path.slice(11);
						return fetch(htmlPath);
					})
					.then((res) => {
						return res.text();
					})
					.then((html) => {
						// We now have the snapshot HTML in text. It needs some additional processing, since the HTML is generated directly from a VSCode extension.
						const article = this.shadowRoot.querySelector(`article`);
						article.innerHTML = new DOMParser()
							.parseFromString(html, "text/html")
							.querySelector(`body`).innerHTML;
						const katex = article.querySelector(
							`script[async][src="https://cdn.jsdelivr.net/npm/katex-copytex@latest/dist/katex-copytex.min.js"]`
						);
						if (katex) {
							katex.remove();
						}
						prependRelativePaths(
							article,
							htmlPath.substr(0, htmlPath.lastIndexOf(`/`)) + `/`
						);

						// <table> postprocessor to wrap them all in a h-scrollable div (the markdown converter doesn’t support this).
						this.shadowRoot.querySelectorAll(`table`).forEach((table) => {
							const wrapper = document.createElement(`div`);
							wrapper.classList.add(`table-wrapper`);
							table.parentNode.insertBefore(wrapper, table);
							wrapper.appendChild(table);
						});

						// Remove any theme toggles.
						this.shadowRoot
							.querySelectorAll(`input.silver-theme-toggle[type="checkbox"]`)
							.forEach((toggle) => {
								toggle.remove();
							});

						// Update URL. Remove .html extension.
						document.title = `${name} | erlija`;
						const fragment = window.location.hash;
						history.pushState(null, ``, `/snapshots/${name}${fragment}`);

						// Display the essay as soon as fonts are loaded!
						document.fonts.ready.then(() => {
							this.classList.add(`loaded`);
							requestAnimationFrame(() => {
								// Process and jump to any ID fragment. Must be done after transition has begun, or else opacity is still 0.
								if (fragment.length > 0) {
									const fragmentElement = this.shadowRoot.getElementById(
										fragment.slice(1)
									);
									if (fragmentElement) {
										// For some reason, this setTimeout is still necessary even with requestAnimationFrame.
										setTimeout(() => {
											fragmentElement.scrollIntoView();
										}, 0);
									}
								}
								resolve();
							});
						});
					});
			});
		}
	}
);
