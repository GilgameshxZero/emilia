import { registerComponent } from "../../scripts/component.js";

registerComponent(
	`noscript`,
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
