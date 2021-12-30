import { registerComponent } from "../components.js";
import { updateUrlWithScene } from "../url.js";
import { listSnapshots } from "../snapshots.js";
import "./marker.js";
import "./sunflower.js";
import "./erlija.js";

registerComponent(
	`map`,
	class extends HTMLElement {
		constructor() {
			super();
			this.onscroll = document.querySelector(`emilia-sunset`).onProgress;
		}

		onComponentDOMContentLoaded() {
			const snapshotsContainer = this.shadowRoot.querySelector(`div`);
			listSnapshots().forEach((snapshot) => {
				const marker = document.createElement(`emilia-marker`);
				marker.setAttribute(`name`, snapshot.name);
				const title = document.createElement(`span`);
				title.setAttribute(`slot`, `title`);
				title.textContent = snapshot.title.toLowerCase();
				const date = document.createElement(`span`);
				date.setAttribute(`slot`, `date`);
				date.textContent = snapshot.date.toLowerCase();
				marker.appendChild(title);
				marker.appendChild(date);
				snapshotsContainer.appendChild(marker);
			});
		}

		// Transitions in map scene.
		sceneIn() {
			this.componentLoad.then(() => {
				const sunflower = this.shadowRoot.querySelector(`emilia-sunflower`);
				Promise.all([
					sunflower.componentLoad,
					...Array.from(this.shadowRoot.querySelectorAll(`emilia-marker`)).map(
						(marker) => {
							return marker.componentLoad;
						}
					)
				]).then(() => {
					updateUrlWithScene(this);

					// Scroll to match progress on sunset.
					const progress = getComputedStyle(
						document.querySelector(`emilia-sunset`)
					).getPropertyValue(`--progress`);
					this.scrollTop = progress * (this.scrollHeight - this.clientHeight);

					document.body.setAttribute(`active`, ``);
					sunflower.setTransition(`erlija`, this);
				});
			});
		}

		// Transition out called by marker.
		sceneOut(snapshotName) {
			document.body.removeAttribute(`active`);
			document.querySelector(`emilia-sunset`).setAttribute(`scene`, `snapshot`);
			const snapshot = document.createElement(`emilia-snapshot`);
			document.body.appendChild(snapshot);
			snapshot.loadSnapshot(snapshotName);
			setTimeout(() => {
				document.body.removeChild(this);
				snapshot.sceneIn();
			}, 500);
		}
	}
);
