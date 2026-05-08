import { registerComponent } from "../../scripts/component.js";

registerComponent(
	`snapshot-menu-item`,
	`erlija-past`,
	class extends HTMLElement {
		onDomLoad() {
			this.subcomponentLoad = new Promise((resolve) => {
				this.resourceLoad.then(() => {
					this.classList.add(`loaded`);
					resolve();
				});
			});
		}
	}
);
