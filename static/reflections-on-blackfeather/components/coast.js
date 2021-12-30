import { registerComponent } from "../components.js";
import { listSnapshots } from "../snapshots.js";
import "./neon.js";
import "./lighthouse.js";

registerComponent(
	`coast`,
	class extends HTMLElement {
		constructor() {
			super();
		}

		onComponentDOMContentLoaded() {
			const lighthousesContainer = this.shadowRoot.querySelector(
				`div[name="lighthouses"]`
			);
			listSnapshots().forEach((snapshot) => {
				const lighthouse = document.createElement(`emilia-lighthouse`);
				lighthouse.setAttribute(`name`, snapshot.name);
				const title = document.createElement(`span`);
				title.setAttribute(`slot`, `title`);
				title.textContent = snapshot.title;
				const date = document.createElement(`span`);
				date.setAttribute(`slot`, `date`);
				date.textContent = snapshot.date;
				lighthouse.appendChild(title);
				lighthouse.appendChild(date);
				lighthousesContainer.appendChild(lighthouse);
			});
			this.componentLoad.then(() => {
				lighthousesContainer.scrollTop = lighthousesContainer.scrollHeight;
			});

			this.shadowRoot
				.querySelector(`emilia-neon[name="arx"]`)
				.addEventListener(`click`, () => {
					document.querySelector(`emilia-underlay`).toArx();
				});
		}
	}
);
