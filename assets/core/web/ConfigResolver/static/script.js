console.log("Script loaded successfully!");

document.addEventListener('DOMContentLoaded', function () {
    // 从页面中获取当前编辑的文件路径
    const fileInput = document.querySelector('input[name="file"]');
    if (fileInput) {
        currentFile = fileInput.value;
    }

    // 启动长轮询
    startPolling();

    // 绑定所有事件
    bindEvents();
});

function startPolling() {
    checkUpdates();
}

function checkUpdates() {
    fetch("/api/check-updates")
        .then(response => {
            if (!response.ok) throw new Error("Network response was not ok");
            const contentType = response.headers.get('content-type');
            if (!contentType || !contentType.includes('application/json')) {
                throw new TypeError("Response is not JSON");
            }
            return response.json();
        })
        .then(data => {
            // 确保data.refresh存在
            const refreshType = data.refresh || "none";

            switch (refreshType) {
                case "list":
                    refreshFileList();
                    break;
                case "editor":
                    if (currentFile) refreshEditor();
                    break;
                // "none"时不执行任何操作
            }

            // 无论是否有更新，3秒后再次检查
            setTimeout(checkUpdates, 3000);
        })
        .catch(error => {
            console.error('Polling error:', error);
            showNotification("连接中断，正在尝试重新连接...", "error");

            // 出错时5秒后重试
            const retryDelay = Math.min(30000, 5000 * Math.pow(2, retryCount++));
            setTimeout(checkUpdates, 5000);
        });
}
let retryCount = 0;
// 刷新文件列表
function refreshFileList() {
    fetch("/api/files")
        .then(response => response.text())
        .then(html => {
            const fileListContainer = document.querySelector('.file-list');
            if (fileListContainer) {
                fileListContainer.innerHTML = html;
                // 重新绑定文件点击事件
                bindFileClickEvents();
            }
        });
}

// 刷新编辑器内容
function refreshEditor() {
    fetch("/api/editor-content")
        .then(response => response.text())
        .then(html => {
            // 更精确地定位要替换的元素
            const editorContent = document.querySelector('.config-form-container');
            if (editorContent) {
                // 创建一个临时容器来解析HTML
                const temp = document.createElement('div');
                temp.innerHTML = html;

                // 只替换内部内容，保留外层容器
                editorContent.innerHTML = temp.querySelector('.config-form-container').innerHTML;

                // 重新绑定事件
                bindEditorEvents();
            }
        });
}

function bindFileClickEvents() {
    // 找到所有文件项（动态加载的元素）
    document.querySelectorAll('.file-item').forEach(item => {
        // 为每个项绑定点击事件
        item.addEventListener('click', function (e) {
            e.preventDefault(); // 阻止默认跳转行为

            // 从 onclick 属性中提取文件路径（如 '/edit?file=path/to/file.xml'）
            const filePath = this.getAttribute('onclick').match(/file=([^']+)/)[1];

            // 更新当前文件（全局变量）
            currentFile = decodeURIComponent(filePath);

            // 修改浏览器URL（无刷新）
            window.history.pushState(null, '', `/edit?file=${filePath}`);

            // 刷新编辑器内容
            refreshEditor();
        });
    });
}

function bindEditorEvents() {
    // 保存按钮事件
    const saveBtn = document.querySelector('.button-container button');
    if (saveBtn) {
        saveBtn.addEventListener('click', saveAllChanges);
    }

    // 表单提交事件
    const addForm = document.querySelector('.add-config-form form');
    if (addForm) {
        addForm.addEventListener('submit', function (e) {
            e.preventDefault();
            const formData = new FormData(this);

            fetch('/add', {
                method: 'POST',
                body: new URLSearchParams(formData)
            })
                .then(response => {
                    if (response.ok) {
                        refreshEditor();
                        this.reset();
                    } else {
                        alert('Failed to add new item');
                    }
                });
        });
    }
}

function bindEvents() {
    bindFileClickEvents();
    bindEditorEvents();
}



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

window.addEventListener('popstate', function () {
    if (location.pathname === '/edit') {
        const fileParam = new URLSearchParams(location.search).get('file');
        if (fileParam) {
            currentFile = decodeURIComponent(fileParam);
            refreshEditor();
        }
    } else {
        refreshFileList();
    }
});


function shutdownServer() {
    fetch("/api/exit")
        .then(response => response.text())
        .then(data => {
            alert(data); // 显示 "Server is shutting down. Goodbye!"
        })
        .catch(err => {
            alert("无法连接到服务器");
        });
}