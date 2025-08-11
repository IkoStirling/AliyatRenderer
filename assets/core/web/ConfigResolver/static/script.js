function saveAllChanges() {
    const formData = new FormData();
    formData.append('file', document.getElementById('current_file').value);

    // 收集所有配置项
    document.querySelectorAll('.config-table input, .config-table select').forEach(input => {
        formData.append(input.name, input.value);
    });

    fetch('/save', {
        method: 'POST',
        body: new URLSearchParams(formData)
    }).then(response => {
        if (response.ok) location.reload();
        else alert('Save failed');
    });
}