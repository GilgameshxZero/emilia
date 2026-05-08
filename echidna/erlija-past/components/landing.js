import { registerComponent } from "../../scripts/component.js";
import "./background.js";
import "./timeline.js";
import "./snapshot.js";
import "./map.js";

registerComponent(
	`landing`,
	`erlija-past`,
	class extends HTMLElement {
		onDomLoad() {
			// Save this element to be used later.
			this.background = this.shadowRoot.querySelector(`emilia-background`);

			// Set favicon. Title is set by specific subcomponents.
			const favicon = document.querySelector(`link[rel="icon"]`);
			favicon.href = `erlija-past/assets/icon.svg`;
			favicon.type = `image/svg+xml`;

			// We are subcomponentLoaded when the timeline has subcomponentLoaded.
			this.subcomponentLoad = new Promise((resolve) => {
				this.resourceLoad.then(() => {
					// Based on the URI, redirect to either timeline, map, or snapshot.
					const path = window.location.pathname;
					if (path === `/map`) {
						this.toMap(resolve);
					} else if (path.startsWith(`/snapshots/`)) {
						this.toSnapshot(path.slice(11), resolve);
					} else {
						this.toTimeline(resolve);
					}
				});
			});
			this.subcomponentLoad.then(() => {
				this.classList.add(`loaded`);
			});

			// Pressing the back button loads a subcomponent based on the new url.
			window.addEventListener(`popstate`, () => {
				const path = window.location.pathname;
				if (path === `/map`) {
					this.toMap();
				} else if (path.startsWith(`/snapshots/`)) {
					this.toSnapshot(path.slice(11));
				} else {
					this.toTimeline();
				}
			});
		}

		toTimeline(onTransitionEnd = null) {
			// This is a no-op if no event listener was set.
			window.removeEventListener(`scroll`, this.background.onScroll);

			// Move background immediately, as the background moves slower and this feels more responsive.
			this.background.toTimeline();

			this.promiseCurrentTransitionEnd().then(() => {
				const timeline = document.createElement(`emilia-timeline`);
				timeline.classList.add(`disabled`);
				this.shadowRoot.appendChild(timeline);
				timeline.resourceLoad.then(() => {
					timeline.subcomponentLoad.then(() => {
						timeline.classList.remove(`disabled`);
						window.scrollTo(0, 0);
						if (onTransitionEnd) {
							onTransitionEnd();
						}
					});
				});
			});
		}

		// Called by timeline.
		toSnapshot(name, onTransitionEnd = null) {
			this.background.resetScroll();
			this.background.toSnapshot();

			this.promiseCurrentTransitionEnd().then(() => {
				const snapshot = document.createElement(`emilia-snapshot`);
				snapshot.classList.add(`disabled`);
				this.shadowRoot.appendChild(snapshot);
				snapshot.resourceLoad.then(() => {
					snapshot.subcomponentLoad.then(() => {
						snapshot.classList.remove(`disabled`);
						snapshot.setSnapshotName(name);

						// setSnapshotName resets subcomponentLoad, so wait on it again.
						snapshot.subcomponentLoad.then(() => {
							window.scrollTo(0, 0);

							// TODO: This may break if the user scrolls frequently, and the transitionend event on background never fires, causing the background to lag in snapshots.
							window.addEventListener(`scroll`, this.background.onScroll);
							if (onTransitionEnd) {
								onTransitionEnd();
							}
						});
					});
				});
			});
		}

		// Called by timeline.
		toMap(onTransitionEnd = null) {
			this.background.toMap();

			this.promiseCurrentTransitionEnd().then(() => {
				const map = document.createElement(`emilia-map`);
				map.classList.add(`disabled`);
				this.shadowRoot.appendChild(map);
				map.resourceLoad.then(() => {
					map.subcomponentLoad.then(() => {
						map.classList.remove(`disabled`);
						if (onTransitionEnd) {
							onTransitionEnd();
						}
					});
				});
			});
		}

		// Returns a promise that resolves when the transition ends for the current element unload, or immediately if there is no current element.
		promiseCurrentTransitionEnd() {
			const current = this.shadowRoot.querySelector(
				`emilia-timeline, emilia-snapshot, emilia-map`
			);
			return new Promise((resolve) => {
				if (!current) {
					resolve();
				}
				current.classList.add(`disabled`);
				current.classList.remove(`loaded`);
				current.addEventListener(`transitionend`, () => {
					current.remove();
					resolve();
				});
			});
		}
	}
);
