import { registerComponent } from "../../scripts/component.js";
import "./icon.js";
import "./noscript.js";

registerComponent(
	`map`,
	`erlija-past`,
	class extends HTMLElement {
		onDomLoad() {
			document.title = `map | erlija`;
			history.pushState(null, ``, `/map`);

			// Click handler to go back to timeline.
			const icon = this.shadowRoot.querySelector(`emilia-icon`);
			icon.addEventListener(`click`, () => {
				this.parentNode.host.toTimeline();
			});

			const noscript = this.shadowRoot.querySelector(`emilia-noscript`);
			noscript.addEventListener(`click`, () => {
				window.location.href = `/api/noscript.html`;
			});

			this.subcomponentLoad = new Promise((resolve) => {
				Promise.all([
					this.resourceLoad,
					icon.resourceLoad,
					noscript.resourceLoad
				]).then(() => {
					Promise.all([icon.subcomponentLoad, noscript.subcomponentLoad]).then(
						() => {
							this.classList.add(`loaded`);
							resolve();
						}
					);
				});
			});
		}
	}
);
