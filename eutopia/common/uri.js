// Provides utility for resolving search params to a scene.
export default function sceneFromUri(uri) {
	const queryParams = new URLSearchParams(uri);
	const sceneName = queryParams.get(`scene`) || `map`;
	const scene = document.createElement(`eutopia-${sceneName}`);

	// Attach scene to DOM immediately to begin loading of subcomponents.
	document.body.appendChild(scene);
	document.querySelector(`eutopia-sunset`).setAttribute(`scene`, sceneName);

	if (sceneName === `essay`) {
		scene.setEssay(queryParams.get(`essay`));
	}
	return scene;
}
