local M = {}

local content_win
local content_buf
local prompt_win
local prompt_buf

M.sideWin = function()
    M.create_side_win()
    M.setup_auto_close()
    M.on_submit()
end

M.create_side_win = function()
    content_buf = vim.api.nvim_create_buf(false, true)
    local width = math.floor(vim.o.columns * 0.5)
    local height = math.floor(vim.o.lines * 0.8)
    local content_opts = {
        split = "right",
        width = width,
        height = height,
    }
    content_win = vim.api.nvim_open_win(content_buf, true, content_opts)
    vim.api.nvim_buf_set_lines(content_buf, 0, -1, false, {
        "This is the content buffer.",
        "You can fill it with search results or text.",
        "",
    })
    vim.api.nvim_set_option_value("wrap", true, { win = content_win })
    vim.api.nvim_set_option_value("linebreak", true, { win = content_win })
    vim.api.nvim_set_option_value("breakindent", true, { win = content_win })

    prompt_buf = vim.api.nvim_create_buf(false, true)
    local prompt_opts = {
        split = "below",
        width = width,
        height = vim.o.lines - height,
    }
    prompt_win = vim.api.nvim_open_win(prompt_buf, true, prompt_opts)
    vim.api.nvim_buf_set_lines(prompt_buf, 0, -1, false, { "Prompt: " })
    vim.api.nvim_set_option_value("wrap", true, { win = prompt_win })
    vim.api.nvim_set_option_value("linebreak", true, { win = prompt_win })
    vim.api.nvim_set_option_value("breakindent", true, { win = prompt_win })

    vim.api.nvim_win_set_cursor(prompt_win, { 1, 8 }) -- After "Prompt: "
    vim.cmd("startinsert")
end

M.set_content_text = function(content)
    if content_buf == nil then
        vim.notify(content)
        return
    end
    local last_line = vim.api.nvim_buf_line_count(content_buf)
    if (content == "Processing...") then
        vim.api.nvim_buf_set_lines(content_buf, last_line, -1, false, { content })
    else
        local result = "BOT: " .. content
        local lines = {}
        for line in result:gmatch("[^\n]+") do
            table.insert(lines, line)
        end
        table.insert(lines, "")
        vim.api.nvim_buf_set_lines(content_buf, last_line - 1, -1, false, lines)
    end
end

M.set_user_content = function(prompt)
    local last_line = vim.api.nvim_buf_line_count(content_buf)
    local content = "You: " .. prompt
    vim.api.nvim_buf_set_lines(content_buf, last_line, -1, false, { content })
end

M.close_windows = function()
    if vim.api.nvim_win_is_valid(content_win) then
        vim.api.nvim_win_close(content_win, true)
    end
    if vim.api.nvim_win_is_valid(prompt_win) then
        vim.api.nvim_win_close(prompt_win, true)
    end
    if vim.api.nvim_buf_is_valid(content_buf) then
        vim.api.nvim_buf_delete(content_buf, { force = true })
    end
    if vim.api.nvim_buf_is_valid(prompt_buf) then
        vim.api.nvim_buf_delete(prompt_buf, { force = true })
    end
end

M.setup_auto_close = function()
    vim.api.nvim_create_autocmd("WinClosed", {
        pattern = { tostring(content_win), tostring(prompt_win) },
        callback = M.close_windows,
        once = true, -- Run only once to avoid repeated triggers
    })
end

M.on_submit = function()
    vim.keymap.set("i", "<CR>", function()
        local lines = vim.api.nvim_buf_get_lines(prompt_buf, 0, -1, false)
        local input = table.concat(lines, "\n"):sub(8) --Prompt prefix removed

        vim.schedule(function()
            M.set_user_content(input)
        end)
        vim.schedule(function()
            vim.api.nvim_buf_set_lines(prompt_buf, 0, -1, false, { "Prompt:" })
        end)

        vim.api.nvim_exec_autocmds("User", {
            pattern = "OnPromptSubmit",
            data = { input = input },
        })
    end, { buffer = prompt_buf, noremap = true, silent = true, expr = true })
end

return M
