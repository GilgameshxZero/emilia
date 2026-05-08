import { registerComponent } from "../../scripts/component.js";
import "./snapshot-menu-item.js";

registerComponent(
	`snapshot-menu`,
	`erlija-past`,
	class extends HTMLElement {
		onDomLoad() {
			// This component is basically immediately subcomponentLoaded.
			this.subcomponentLoad = new Promise((resolve) => {
				this.resourceLoad.then(() => {
					resolve();
				});
			});
		}

		// Each menu reset creates a new subcomponentLoad Promise to resolve.
		onMenuReset(snapshots) {
			// Unload all menu items, and the menu itself.
			const unloadThis = new Promise((resolve) => {
				if (this.classList.contains(`loaded`)) {
					this.classList.remove(`loaded`);
					this.addEventListener(
						`transitionend`,
						() => {
							resolve();
						},
						{ once: true }
					);
				} else {
					resolve();
				}
			});

			this.subcomponentLoad = new Promise((resolve) => {
				Promise.all([
					unloadThis,
					...[
						...this.shadowRoot.querySelectorAll(`emilia-snapshot-menu-item`)
					].map((snapshotMenuItem) => {
						return new Promise((resolve) => {
							snapshotMenuItem.addEventListener(`transitionend`, (event) => {
								// For some reason, we also get transitionend events on background-color, so need to filter those out.
								if (event.propertyName === `opacity`) {
									snapshotMenuItem.remove();
									resolve();
								}
							});
							snapshotMenuItem.classList.remove(`loaded`);
						});
					})
				]).then(() => {
					// Construct new menu items.
					for (let idx = snapshots.length - 1; idx >= 0; idx--) {
						const snapshot = snapshots[idx];
						const snapshotMenuItem = document.createElement(
							`emilia-snapshot-menu-item`
						);
						snapshotMenuItem.setAttribute(`name`, snapshot.name);
						const title = document.createElement(`span`);
						title.setAttribute(`slot`, `title`);
						title.textContent = snapshot.title;
						snapshotMenuItem.appendChild(title);
						const date = document.createElement(`span`);
						date.setAttribute(`slot`, `date`);
						date.textContent = new Date(snapshot.date).toLocaleDateString(
							`en-US`,
							{
								year: `numeric`,
								month: `long`,
								day: `numeric`
							}
						);
						snapshotMenuItem.appendChild(date);
						this.shadowRoot.appendChild(snapshotMenuItem);
					}

					// Only resolve subcomponentLoad after all menu items have subcomponentLoaded.
					const subcomponents = [
						...this.shadowRoot.querySelectorAll(`emilia-snapshot-menu-item`)
					];
					Promise.all(
						subcomponents.map((subcomponent) => {
							return subcomponent.resourceLoad;
						})
					).then(() => {
						Promise.all(
							subcomponents.map((subcomponent) => {
								return subcomponent.subcomponentLoad;
							})
						).then(() => {
							this.classList.add(`loaded`);
							resolve();
						});
					});
				});
			});
		}
	}
);
