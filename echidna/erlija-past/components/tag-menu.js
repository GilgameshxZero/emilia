import { registerComponent } from "../../scripts/component.js";
import "./tag-menu-item.js";

registerComponent(
	`tag-menu`,
	`erlija-past`,
	class extends HTMLElement {
		onDomLoad() {
			this.parentHost = this.getRootNode().host;

			// Resolve subcomponentLoad  as soon as all menu items have been created and subcomponentLoaded.
			// This component cannot be reset.
			this.subcomponentLoad = new Promise((resolve) => {
				Promise.all([this.parentHost.snapshotsLoad, this.resourceLoad]).then(
					() => {
						Object.keys(this.parentHost.tags)
							.sort((a, b) => {
								return this.parentHost.tags[a].date >
									this.parentHost.tags[b].date
									? -1
									: 1;
							})
							.map((tag, idx) => {
								const tagMenuItem =
									document.createElement(`emilia-tag-menu-item`);
								tagMenuItem.setAttribute(`tag`, tag);
								const name = document.createElement(`span`);
								name.setAttribute(`slot`, `name`);
								name.textContent = this.parentHost.tags[tag].name;
								tagMenuItem.appendChild(name);
								const tagline = document.createElement(`span`);
								tagline.setAttribute(`slot`, `tagline`);
								tagline.textContent = this.parentHost.tags[tag].tagline;
								tagMenuItem.appendChild(tagline);
								this.shadowRoot.appendChild(tagMenuItem);
								tagMenuItem.addEventListener(`click`, () => {
									this.onTagMenuItemClick(tagMenuItem);
								});
								return tagMenuItem.resourceLoad;
							});

						// Set default selected tag as the most recent one.
						const initialSelectedMenuItem =
							this.shadowRoot.querySelector(`emilia-tag-menu-item`);
						initialSelectedMenuItem.toggleAttribute(`selected`);
						this.parentHost.onTagSelect(
							initialSelectedMenuItem.getAttribute(`tag`)
						);

						// Wait for item subcomponentLoads.
						const subcomponents = [
							...this.shadowRoot.querySelectorAll(`emilia-tag-menu-item`)
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
					}
				);
			});
		}

		onTagMenuItemClick(tagMenuItem) {
			if (!tagMenuItem.hasAttribute(`selected`)) {
				this.shadowRoot
					.querySelector(`emilia-tag-menu-item[selected]`)
					.toggleAttribute(`selected`);
				tagMenuItem.toggleAttribute(`selected`);
				this.parentHost.onTagSelect(tagMenuItem.getAttribute(`tag`));
			}

			// New selected tag must be set after JS animations are trigged on the non-selected elements.
			if (
				window.innerWidth < window.innerHeight &&
				(this.hasAttribute(`expanded`) || tagMenuItem.hasAttribute(`selected`))
			) {
				// This animation must be set before the attribute is toggled, or else the CSS height will take precedence. Disable pointer events while transitioning.
				if (!this.hasAttribute(`expanded`)) {
					// Expanding animation: <https://css-tricks.com/using-css-transitions-auto-dimensions/>.
					this.shadowRoot
						.querySelectorAll(`emilia-tag-menu-item:not([selected])`)
						.forEach((menuItem) => {
							const menuItemHeight = menuItem.scrollHeight;
							menuItem.style.height = `${menuItemHeight}px`;
							menuItem.addEventListener(
								`transitionend`,
								() => {
									menuItem.style.height = null;
								},
								{ once: true }
							);
						});
				} else {
					// Collapsing animation.
					this.shadowRoot
						.querySelectorAll(`emilia-tag-menu-item:not([selected])`)
						.forEach((menuItem) => {
							const menuItemHeight = menuItem.scrollHeight;
							// Temporarily overwrite CSS height 0 in the next animation frame.
							menuItem.style.height = `auto`;
							menuItem.style.transition = `none`;
							requestAnimationFrame(() => {
								menuItem.style.height = `${menuItemHeight}px`;
								menuItem.style.transition = null;
								requestAnimationFrame(() => {
									// Transition height back to CSS 0.
									menuItem.style.height = null;
								});
							});
						});
				}
				this.toggleAttribute(`expanded`);
			}
		}
	}
);
