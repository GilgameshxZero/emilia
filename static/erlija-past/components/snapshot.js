import { prependRelativePaths, registerComponent } from "../component.js";
import { updateUrlWithScene } from "../url.js";
import "./sunflower.js";

registerComponent(
	`snapshot`,
	class extends HTMLElement {
		// Begins fetching of essay; must be called before sceneIn.
		loadSnapshot(name) {
			this.snapshotName = name;
			this.snapshotInnerLoad = new Promise((resolve, reject) => {
				fetch(`snapshots/${name}.html`)
					.then((res) => {
						return res.text();
					})
					.then((text) => {
						resolve(text);
					})
					.catch((reason) => {
						reject(reason);
					});
			});
		}

		// Transition in essay scene, once it has been loaded.
		sceneIn() {
			// At this point, shadowRoot may not exist yet.
			Promise.all([
				this.snapshotInnerLoad,
				// Additionally wait for load promise here, so that both fetch and load are fulfilled.
				this.componentLoad
			]).then((result) => {
				// Now, shadowRoot must exist, and so must the sunflower have been constructed (and thus have componentLoad).
				const sunflowers = this.shadowRoot.querySelectorAll(`emilia-sunflower`);
				Promise.all(
					Array.from(sunflowers).map((sunflower) => {
						return sunflower.componentLoad;
					})
				).then(() => {
					updateUrlWithScene(this);

					// Sunflower is also ready.
					const fragmentClone = new DOMParser()
						.parseFromString(result[0], "text/html")
						.querySelector(`template`)
						.content.cloneNode(true);
					prependRelativePaths(fragmentClone, "../snapshots/");
					this.shadowRoot.querySelector(`article`).appendChild(fragmentClone);

					// Process and jump to any ID fragment.
					const fragment = window.location.hash;
					if (fragment.length > 0) {
						this.shadowRoot.querySelector(fragment).scrollIntoView();
					}

					// Display the essay as soon as fonts are loaded!
					document.fonts.ready.then(() => {
						sunflowers.forEach((sunflower) => {
							sunflower.setTransition(`map`, this);
						});
						document.body.setAttribute(`active`, ``);
					});
				});
			});
		}
	}
);
