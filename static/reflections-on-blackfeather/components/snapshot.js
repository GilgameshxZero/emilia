import { prependRelativePaths, registerComponent } from "../components.js";

registerComponent(
	`snapshot`,
	class extends HTMLElement {
		constructor() {
			super();
		}

		loadSnapshot(snapshotName) {
			return new Promise((resolve, reject) => {
				fetch(`snapshots/${snapshotName}.html`)
					.then((res) => {
						return res.text();
					})
					.catch((reason) => {
						reject(reason);
					})
					.then((text) => {
						const fragmentClone = new DOMParser()
							.parseFromString(text, `text/html`)
							.querySelector(`template`)
							.content.cloneNode(true);
						prependRelativePaths(fragmentClone, `../snapshots/`);
						const article = this.shadowRoot.querySelector(`article`);
						article.innerHTML = ``;
						article.appendChild(fragmentClone);
					});
			});
		}
	}
);
