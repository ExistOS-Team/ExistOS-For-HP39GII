---------------------
-- "KhiCAS" : TI-Nspire Giac UI in Nspire-Lua
-- Version 1.05 - 18/06/2014
-- GPL v3 License
-- http://tiplanet.org/forum/viewtopic.php?f=43&t=14800
---------------------
-- Giac CAS engine by Bernard Parisse
-- http://www-fourier.ujf-grenoble.fr/~parisse/
---------------------
-- The UI is highly based on Xavier 'critor' Andréani's "SuperSpire" 
-- See http://tiplanet.org/forum/viewtopic.php?t=13851&p=157015
-------------------
-- ETK GUI Lib by Jim Bauwens and Adrien "Adriweb" Bertrand
-- Additions/mods by Xavier
-- some more little changes by Adrien
-- (borders, horiz. lines, readonly, copy/paste, errHandler etc.)
-------------------

platform.apilevel = '2.0'

hasGiac = pcall(nrequire, "luagiac")
if not hasGiac then
	luagiac = { caseval = function(str) return math.evalStr(str) end }
	print("Giac module not loaded ! Fallback on Nspire's math engine.")
end

-- localized useful functions

local mathmin = math.min
local mathmax = math.max
local mathrandom = math.random

ts = 0.0


-- ETK Stuff

--------------------------------------------------------------------- View: Widgets & Events manager

defaultFocus = nil

View = class()

function View:init(window)
	self.window = window
	self.widgetList = {}
	self.focusList = {}
	self.currentFocus = 0
	self.currentCursor = "default"

	-- Previous location of mouse pointer
	self.prev_mousex = 0
	self.prev_mousey = 0
end


function View:invalidate()
	self.window:invalidate()
end

function View:setCursor(cursor)
	if cursor ~= self.currentCursor then
		self.currentCursor = cursor
		self:invalidate()
	end
end


function View:add(o)
	table.insert(self.widgetList, o)
	self:repos(o)
	if o.acceptsFocus then
		table.insert(self.focusList, 1, o)
		if self.currentFocus > 0 then
			self.currentFocus = self.currentFocus + 1
		end
	end
	return o
end

function View:remove(o)
	if self:getFocus() == o then
		o:releaseFocus()
	end
	local i = 1
	local f = 0
	local oldf
	while i <= #self.focusList do
		if self.focusList[i] == o then
			f = i
		end
		i = i + 1
	end
	if f > 0 then
		if self:getFocus() == o then
			self:tabForward()
		end
		table.remove(self.focusList, f)
		if self.currentFocus > f then
			self.currentFocus = self.currentFocus - 1
		end
	end
	f = 0
	i = 1
	while i <= #self.widgetList do
		if self.widgetList[i] == o then
			f = i
		end
		i = i + 1
	end
	if f > 0 then
		table.remove(self.widgetList, f)
	end
	--    table.remove(self.widgetList, o)
	--    table.remove(self.focusList, o)
end

function View:repos(o)
	local x = o.x
	local y = o.y
	local w = o.w
	local h = o.h
	if o.hConstraint == "right" then
		x = scrWidth - o.w - o.dx1
	elseif o.hConstraint == "center" then
		x = (scrWidth - o.w + o.dx1) / 2
	elseif o.hConstraint == "justify" then
		w = scrWidth - o.x - o.dx1
	end
	if o.vConstraint == "bottom" then
		y = scrHeight - o.h - o.dy1
	elseif o.vConstraint == "middle" then
		y = (scrHeight - o.h + o.dy1) / 2
	elseif o.vConstraint == "justify" then
		h = scrHeight - o.y - o.dy1
	end
	o:repos(x, y)
	o:resize(w, h)
end

function View:resize()
	for _, o in ipairs(self.widgetList) do
		self:repos(o)
	end
end

function View:hide(o)
	if o.visible then
		o.visible = false
		self:releaseFocus(o)
		if o:contains(self.prev_mousex, self.prev_mousey) then
			o:onMouseLeave(o.x - 1, o.y - 1)
		end
		self:invalidate()
	end
end

function View:show(o)
	if not o.visible then
		o.visible = true
		if o:contains(self.prev_mousex, self.prev_mousey) then
			o:onMouseEnter(self.prev_mousex, self.prev_mousey)
		end
		self:invalidate()
	end
end

function View:getFocus()
	if self.currentFocus == 0 then
		return nil
	end
	return self.focusList[self.currentFocus]
end

function View:setFocus(obj)
	if self.currentFocus ~= 0 then
		if self.focusList[self.currentFocus] == obj then
			return
		end
		self.focusList[self.currentFocus]:releaseFocus()
	end
	self.currentFocus = 0
	for i = 1, #self.focusList do
		if self.focusList[i] == obj then
			self.currentFocus = i
			obj:setFocus()
			self:invalidate()
			break
		end
	end
end

function View:releaseFocus(obj)
	if self.currentFocus ~= 0 then
		if self.focusList[self.currentFocus] == obj then
			self.currentFocus = 0
			obj:releaseFocus()
			self:invalidate()
		end
	end
end

function View:sendStringToFocus(str)
	local o = self:getFocus()
	if not o then
		o = defaultFocus
		self:setFocus(o)
	end
	if o then
		if o.visible then
			if o:addString(str) then
				self:invalidate()
			else
				o = nil
			end
		end
	end

	if not o then -- look for a default handler
		for _, o in ipairs(self.focusList) do
			if o.visible then
				if o:addString(str) then
					self:setFocus(o)
					self:invalidate()
					break
				end
			end
		end
	end
end


function View:backSpaceHandler()
	-- Does the focused widget accept BackSpace?
	local o = self:getFocus()
	if o then
		if o.visible and o.acceptsBackSpace then
			o:backSpaceHandler()
			self:setFocus(o)
			self:invalidate()
		else
			o = nil
		end
	end
	if not o then -- look for a default handler
		for _, o in ipairs(self.focusList) do
			if o.visible and o.acceptsBackSpace then
				o:backSpaceHandler()
				self:setFocus(o)
				self:invalidate()
				break;
			end
		end
	end
end


function View:tabForward()
	local nextFocus = self.currentFocus + 1
	if nextFocus > #self.focusList then
		nextFocus = 1
	end
	self:setFocus(self.focusList[nextFocus])
	if self:getFocus() then
		if not self:getFocus().visible then
			self:tabForward()
		end
	end
	self:invalidate()
end


function View:tabBackward()
	local nextFocus = self.currentFocus - 1
	if nextFocus < 1 then
		nextFocus = #self.focusList
	end
	self:setFocus(self.focusList[nextFocus])
	if not self:getFocus().visible then
		self:tabBackward()
	end
	self:invalidate()
end


function View:onMouseDown(x, y)
	-- Find a widget that has a mouse down handler and bounds the click point
	for _, o in ipairs(self.widgetList) do
		if o.visible and o.acceptsFocus and o:contains(x, y) then
			self.mouseCaptured = o
			o:onMouseDown(o, window, x - o.x, y - o.y)
			self:setFocus(o)
			self:invalidate()
			return
		end
	end
	if self:getFocus() then
		self:setFocus(nil)
		self:invalidate()
	end
end


function View:onMouseMove(x, y)
	local prev_mousex = self.prev_mousex
	local prev_mousey = self.prev_mousey
	for _, o in ipairs(self.widgetList) do
		local xyin = o:contains(x, y)
		local prev_xyin = o:contains(prev_mousex, prev_mousey)
		if xyin and not prev_xyin and o.visible then
			-- Mouse entered widget
			o:onMouseEnter(x, y)
			self:invalidate()
		elseif prev_xyin and (not xyin or not o.visible) then
			-- Mouse left widget
			o:onMouseLeave(x, y)
			self:invalidate()
		end
	end
	self.prev_mousex = x
	self.prev_mousey = y
end


function View:onMouseUp(x, y)
	local mc = self.mouseCaptured
	if mc then
		self.mouseCaptured = nil
		if mc:contains(x, y) then
			mc:onMouseUp(x - mc.x, y - mc.y)
		else
			--            mc:cancelClick()
		end
	end
end


function View:enterHandler()
	-- Does the focused widget accept Enter?
	local o = self:getFocus()
	if o then
		if o.visible and o.acceptsEnter then
			o:enterHandler()
			self:setFocus(o)
			self:invalidate()
		else
			o = nil
		end
	end
	if not o then -- look for a default handler
		for _, o in ipairs(self.focusList) do
			if o.visible and o.acceptsEnter then
				o:enterHandler()
				self:setFocus(o)
				self:invalidate()
				break;
			end
		end
	end
end

function View:arrowLeftHandler()
	-- Does the focused widget accept ArrowLeft?
	local o = self:getFocus()
	if o then
		if o.visible and o.acceptsArrowLeft then
			o:arrowLeftHandler()
			self:setFocus(o)
			self:invalidate()
		else
			o = nil
		end
	end
	if not o then -- look for a default handler
		for _, o in ipairs(self.focusList) do
			if o.visible and o.acceptsArrowLeft then
				o:arrowLeftHandler()
				self:setFocus(o)
				self:invalidate()
				break;
			end
		end
	end
end

function View:arrowRightHandler()
	-- Does the focused widget accept ArrowRight?
	local o = self:getFocus()
	if o then
		if o.visible and o.acceptsArrowRight then
			o:arrowRightHandler()
			self:setFocus(o)
			self:invalidate()
		else
			o = nil
		end
	end
	if not o then -- look for a default handler
		for _, o in ipairs(self.focusList) do
			if o.visible and o.acceptsArrowRight then
				o:arrowRightHandler()
				self:setFocus(o)
				self:invalidate()
				break;
			end
		end
	end
end

function View:arrowUpHandler()
	-- Does the focused widget accept ArrowUp?
	local o = self:getFocus()
	if o then
		if o.visible and o.acceptsArrowUp then
			o:arrowUpHandler()
			self:setFocus(o)
			self:invalidate()
		else
			o = nil
		end
	end
	if not o then -- look for a default handler
		for _, o in ipairs(self.focusList) do
			if o.visible and o.acceptsArrowUp then
				o:arrowUpHandler()
				self:setFocus(o)
				self:invalidate()
				break;
			end
		end
	end
end

function View:arrowDownHandler()
	-- Does the focused widget accept ArrowDown?
	local o = self:getFocus()
	if o then
		if o.visible and o.acceptsArrowDown then
			o:arrowDownHandler()
			self:setFocus(o)
			self:invalidate()
		else
			o = nil
		end
	end
	if not o then -- look for a default handler
		for _, o in ipairs(self.focusList) do
			if o.visible and o.acceptsArrowDown then
				o:arrowDownHandler()
				self:setFocus(o)
				self:invalidate()
				break;
			end
		end
	end
end

function View:paint(gc)
	local fo = self:getFocus()
	for _, o in ipairs(self.widgetList) do
		if o.visible then
			o:paint(gc, fo == o)
			if fo == o then
				gc:setColorRGB(100, 150, 255)
				gc:drawRect(o.x - 1, o.y - 1, o.w + 1, o.h + 1)
				gc:setPen("thin", "smooth")
				gc:setColorRGB(0)
			end
		end
	end
	cursor.set(self.currentCursor)
end

theView = nil

--------------------------------------------------------------------- Widget

Widget = class()

function Widget:setHConstraints(hConstraint, dx1)
	self.hConstraint = hConstraint
	self.dx1 = dx1
end

function Widget:setVConstraints(vConstraint, dy1)
	self.vConstraint = vConstraint
	self.dy1 = dy1
end

function Widget:init(view, x, y, w, h)
	self.xOrig = x
	self.yOrig = y
	self.view = view
	self.x = x
	self.y = y
	self.w = w
	self.h = h
	self.acceptsFocus = false
	self.visible = true
	self.acceptsEnter = false
	self.acceptsEscape = false
	self.acceptsTab = false
	self.acceptsDelete = false
	self.acceptsBackSpace = false
	self.acceptsReturn = false
	self.acceptsArrowUp = false
	self.acceptsArrowDown = false
	self.acceptsArrowLeft = false
	self.acceptsArrowRight = false
	self.hConstraint = "left"
	self.vConstraint = "top"
end

function Widget:repos(x, y)
	self.x = x
	self.y = y
end

function Widget:resize(w, h)
	self.w = w
	self.h = h
end

function Widget:setFocus()
end

function Widget:releaseFocus()
end

function Widget:contains(x, y)
	return x >= self.x and x <= self.x + self.w
			and y >= self.y and y <= self.y + self.h
end


function Widget:onMouseEnter(x, y)
	-- Implemented in subclasses
end


function Widget:onMouseLeave(x, y)
	-- Implemented in subclasses
end

function Widget:paint(gc, focused)
	-- Implemented in subclasses
end

function Widget:enterHandler()
end

function Widget:escapeHandler()
end

function Widget:tabHandler()
end

function Widget:deleteHandler()
end

function Widget:backSpaceHandler()
end

function Widget:returnHandler()
end

function Widget:arrowUpHandler()
end

function Widget:arrowDownHandler()
end

function Widget:arrowLeftHandler()
end

function Widget:arrowRightHandler()
end

function Widget:onMouseDown(x, y)
end

function Widget:onMouseUp(x, y)
end

--------------------------------------------------------------------- Button widget

Button = class(Widget)

function Button:init(view, x, y, w, h, default, command, shortcut)
	Widget.init(self, view, x, y, w, h)
	-- Button configuration
	self.acceptsFocus = true
	self.acceptsBackspace = false
	self.command = command or function() end -- what to do when pressed
	self.default = default -- is default button when ENTER is pressed
	self.shortcut = shortcut
	-- Current button state
	self.clicked = false
	self.highlighted = false
	self.acceptsEnter = true
end

-- Act on key press on button
function Button:enterHandler()
	if self.acceptsEnter then
		self:command()
	end
end

function Button:escapeHandler()
	if self.acceptsEscape then
		self:command()
	end
end

function Button:tabHandler()
	if self.acceptsTab then
		self:command()
	end
end

function Button:deleteHandler()
	if self.acceptsDelete then
		self:command()
	end
end

function Button:backSpaceHandler()
	if self.acceptsBackSpace then
		self:command()
	end
end

function Button:returnHandler()
	if self.acceptsReturn then
		self:command()
	end
end

function Button:arrowUpHandler()
	if self.acceptsArrowUp then
		self:command()
	end
end

function Button:arrowDownHandler()
	if self.acceptsArrowDown then
		self:command()
	end
end

function Button:arrowLeftHandler()
	if self.acceptsArrowLeft then
		self:command()
	end
end

function Button:arrowRightHandler()
	if self.acceptsArrowRight then
		self:command()
	end
end

function Button:arrowUpHandler()
	if self.acceptsArrowUp then
		self:command()
	end
end

function Button:arrowDownHandler()
	if self.acceptsArrowDown then
		self:command()
	end
end

function Button:onMouseDown(x, y)
	self.clicked = true
	self.highlighted = true
end

function Button:onMouseEnter(x, y)
	theView:setCursor("hand pointer")
	if self.clicked and not self.highlighted then
		self.highlighted = true
	end
end

function Button:onMouseLeave(x, y)
	theView:setCursor("default")
	if self.clicked and self.highlighted then
		self.highlighted = false
	end
end

function Button:cancelClick()
	if self.clicked then
		self.highlighted = false
		self.clicked = false
	end
end

function Button:onMouseUp(x, y)
	self:cancelClick()
	self:command()
end

function Button:addString(str)
	if str == " " or str == self.shortcut then
		self:command()
		return true
	end
	return false
end

--------------------------------------------------------------------- ImgLabel widget

ImgLabel = class(Widget)

function ImgLabel:init(view, x, y, img)
	self.img = image.new(img)
	self.w = image.width(self.img)
	self.h = image.height(self.img)
	Widget.init(self, view, x, y, self.w, self.h, false, command, shortcut)
end

function ImgLabel:paint(gc, focused)
	gc:drawImage(self.img, self.x, self.y)
end

--------------------------------------------------------------------- ImgButton widget

ImgButton = class(Button)

function ImgButton:init(view, x, y, img, command, shortcut)
	self.img = image.new(img)
	self.w = image.width(self.img)
	self.h = image.height(self.img)
	Button.init(self, view, x, y, self.w, self.h, false, command, shortcut)
end

function ImgButton:paint(gc, focused)
	gc:drawImage(self.img, self.x, self.y)
end

--------------------------------------------------------------------- TextButton widget

TextButton = class(Button)

function TextButton:init(view, x, y, text, command, shortcut)
	self.textid = text
	self.text = getLocaleText(text)
	self:resize(0, 0)
	Button.init(self, view, x, y, self.w, self.h, false, command, shortcut)
end

function TextButton:resize(w, h)
	self.text = getLocaleText(self.textid)
	self.w = getStringWidth(self.text) + 5
	self.h = getStringHeight(self.text) + 5
end


function TextButton:paint(gc, focused)
	gc:setColorRGB(223, 223, 223)
	gc:drawRect(self.x + 1, self.y + 1, self.w - 2, self.h - 2)
	gc:setColorRGB(191, 191, 191)
	gc:fillRect(self.x + 1, self.y + 1, self.w - 3, self.h - 3)
	gc:setColorRGB(223, 223, 223)
	gc:drawString(self.text, self.x + 3, self.y + 3, "top")
	gc:setColorRGB(0)
	gc:drawString(self.text, self.x + 2, self.y + 2, "top")
	gc:drawRect(self.x, self.y, self.w - 2, self.h - 2)
end

--------------------------------------------------------------------- vertical scroll bar
VScrollBar = class(Widget)

function VScrollBar:init(view, x, y, w, h)
	self.pos = 10
	self.siz = 10
	Widget.init(self, view, x, y, w, h, false)
end

function VScrollBar:paint(gc, focused)
	gc:setColorRGB(0)
	gc:drawRect(self.x, self.y, self.w, self.h)
	gc:fillRect(self.x + 2, self.y + self.h - (self.h - 4) * (self.pos + self.siz) / 100 - 2, self.w - 3, mathmax(1, (self.h - 4) * self.siz / 100 + 1))
end

--------------------------------------------------------------------- Text widget

TextLabel = class(Widget)

function TextLabel:init(view, x, y, text)
	self:setText(text)
	Widget.init(self, view, x, y, self.w, self.h, false)
end

function TextLabel:resize(w, h)
	self.text = getLocaleText(self.textid)
	self.w = getStringWidth(self.text)
	self.h = getStringHeight(self.text)
end

function TextLabel:setText(text)
	self.textid = text
	self.text = getLocaleText(text)
	self:resize(0, 0)
end

function TextLabel:getText()
	return self.text
end

function TextLabel:paint(gc, focused)
	gc:setColorRGB(0)
	gc:drawString(self.text, self.x, self.y, "top")
end

--------------------------------------------------------------------- editable RichText widget 

RichTextEditor = class(Widget)

function RichTextEditor:init(view, x, y, w, h, text)
	self.editor = D2Editor.newRichText()
	self.readOnly = false
	self:repos(x, y)
	self.editor:setFontSize(fsize)
	self.editor:setFocus(false)
	self.text = text
	self:resize(w, h)
	Widget.init(self, view, x, y, self.w, self.h, true)
	self.acceptsFocus = true
	self.editor:setExpression(text)
	self.editor:setBorder(1)
end

function RichTextEditor:onMouseEnter(x, y)
	theView:setCursor("text")
end

function RichTextEditor:onMouseLeave(x, y)
	theView:setCursor("default")
end

function RichTextEditor:repos(x, y)
	if not self.editor then self = nil; return; end
	self.editor:setBorderColor((showEditorsBorders and 0) or 0xffffff)
	self.editor:move(x+1, y+1)
	Widget.repos(self, x, y)
end

function RichTextEditor:resize(w, h)
	if not self.editor then self = nil; return; end
	self.editor:resize(w-1, h-1)
	Widget.resize(self, w, h)
end

function RichTextEditor:setFocus()
	self.editor:setFocus(true)
end

function RichTextEditor:releaseFocus()
	self.editor:setFocus(false)
end

function RichTextEditor:addString(str)
	local currentText = self.editor:getText() or ""
	self.editor:setText(currentText .. str)
	return true
end


function RichTextEditor:paint(gc, focused)
	--    self.editor:paint(gc)
end

--------------------------------------------------------------------- editable Math widget 

MathEditor = class(RichTextEditor)

-- pretty printed square root characters from MathBoxes take two 'special' characters in returned expression string
-- cursor position in expression returned by getExpression() then does not match cursor position in string after the square root
function string.ulen(s)
	if not s then return 0 else return select(2, s:gsub("[^\128-\193]", "")) end
end
ulen = string.ulen

function MathEditor:init(view, x, y, w, h, text)
	RichTextEditor.init(self, view, x, y, w, h, text)
	self.editor:setBorder(1)
	self.acceptsEnter = true
	self.acceptsBackSpace = true
	self.result = false
	-- add editor focus listener which does: setFocus(getME(editor))
	self.editor:registerFilter({
		arrowLeft = function()
			local _, curpos = self.editor:getExpressionSelection()
			if curpos < 7 then
				on.arrowLeft()
				return true
			end
			return false
		end,
		arrowRight = function()
			local currentText, curpos = self.editor:getExpressionSelection()
			if curpos > ulen(currentText) - 2 then
				on.arrowRight()
				return true
			end
			return false
		end,
		tabKey = function()
			theView:tabForward()
			return true
		end,
		mouseDown = function(x, y)
			theView:onMouseDown(x, y)
			return false
		end,
		backspaceKey = function()
			if (self == fctEditor) then
				self:fixCursor()
				local _, curpos = self.editor:getExpressionSelection()
				if curpos <= 6 then return true end
				return false
			else
				self:backSpaceHandler()
				return true
			end
		end,
		deleteKey = function()
			if (self == fctEditor) then
				self:fixCursor()
				local currentText, curpos = self.editor:getExpressionSelection()
				if curpos >= ulen(currentText) - 1 then return true end
				return false
			else
				self:backSpaceHandler()
				return true
			end
		end,
		enterKey = function()
			self:enterHandler()
			return true
		end,
		returnKey = function()
			theView:enterHandler()
			return true
		end,
		escapeKey = function()
			on.escapeKey()
			return true
		end,
		clearKey = function()
			if self == fctEditor then
				self.editor:setExpression("")
				self:fixContent()
			else
				self:backSpaceHandler()
			end
			return true
		end,
		charIn = function(c)
			if (self == fctEditor) then
				if self.editor:getExpression() then
					self:fixCursor()
				end
				return false
			else
				return self.readOnly
			end
		end
	})
end

function MathEditor:fixContent()
	local currentText = self.editor:getExpressionSelection()
	if not currentText or currentText == "" then
		self.editor:createMathBox()
	end
end

function MathEditor:fixCursor()
	local currentText, curpos, selstart = self.editor:getExpressionSelection()
	local l = ulen(currentText)
	if curpos < 6 or selstart < 6 or curpos > l - 1 or selstart > l - 1 then
		if curpos < 6 then curpos = 6 end
		if selstart < 6 then selstart = 6 end
		if curpos > l - 1 then curpos = l - 1 end
		if selstart > l - 1 then selstart = l - 1 end
		self.editor:setExpression(currentText, curpos, selstart)
	end
end

function MathEditor:getExpression()
	if not self.editor then self = nil; return ""; end
	local rawexpr = self.editor:getExpression()
	local expr = ""
	local n = rawexpr:len()
	local b = 0
	local bs = 0
	local bi = 0
	local status = 0
	local i = 1
	local c
	while i <= n do
		c = rawexpr:sub(i, i)
		if c == "{" then
			b = b + 1
		elseif c == "}" then
			b = b - 1
		end
		if status == 0 then
			if rawexpr:sub(i, i + 5) == "\\0el {" then
				bs = i + 6
				i = i + 5
				status = 1
				bi = b
				b = b + 1
			end
		else
			if b == bi then
				status = 0
				expr = expr .. rawexpr:sub(bs, i - 1)
			end
		end
		i = i + 1
	end
	return expr
end

function MathEditor:setFocus()
	if not self.editor then self = nil; return; end
	self.editor:setFocus(true)
end

function MathEditor:releaseFocus()
	if not self.editor then self = nil; return; end
	self.editor:setFocus(false)
end

function MathEditor:addString(str)
	if not self.editor then self = nil; return; end
	self:fixCursor()
	local currentText, curpos, selstart = self.editor:getExpressionSelection()
	currentText = currentText:usub(1, mathmin(curpos, selstart)) .. str .. currentText:usub(mathmax(curpos, selstart) + 1, ulen(currentText))
	self.editor:setExpression(currentText, mathmin(curpos, selstart) + ulen(str))
	return true
end

function MathEditor:backSpaceHandler()
	backSpaceHandler(self)
end

function MathEditor:enterHandler()
	enterHandler(self)
end

function MathEditor:paint(gc)
	if not self.editor then self = nil; return; end
	if showHLines and not self.result then
		gc:setColorRGB(100, 100, 100)
		local ycoord = self.y - (showEditorsBorders and 0 or 2)
		gc:drawLine(1, ycoord, platform.window:width() - sbv.w - 2, ycoord)
		gc:setColorRGB(0)
	end
end

--------------------------------------------------------------------- events handling
function on.arrowUp()
	if theView:getFocus() == fctEditor then
		on.tabKey()
	else
		on.tabKey()
		if theView:getFocus() ~= fctEditor then on.tabKey() end
	end
	reposView()
end

function on.arrowDown()
	if theView:getFocus() == fctEditor then return end
	on.backtabKey()
	if theView:getFocus() ~= fctEditor then on.backtabKey() end
	reposView()
end

function on.arrowLeft()
	if theView:getFocus() == fctEditor then return end
	on.tabKey()
	reposView()
end

function on.arrowRight()
	if theView:getFocus() == fctEditor then return end
	on.backtabKey()
	reposView()
end

function on.charIn(ch)
	theView:sendStringToFocus(ch)
end

function on.tabKey()
	theView:tabForward()
	reposView()
end

function on.backtabKey()
	theView:tabBackward()
	reposView()
end

function on.escapeKey()
	-- nothing to do ?
end

function on.enterKey()
	theView:enterHandler()
end
on.returnKey = on.enterKey

function on.mouseMove(x, y)
	theView:onMouseMove(x, y)
end

function on.mouseDown(x, y)
	theView:onMouseDown(x, y)
	--		theView:invalidate()
end

function on.mouseUp(x, y)
	theView:onMouseUp(x, y)
end

function initFontGC(gc)
	gc:setFont(font, style, fsize)
end

function getStringHeightGC(text, gc)
	initFontGC(gc)
	return gc:getStringHeight(text)
end

function getStringHeight(text)
	return platform.withGC(getStringHeightGC, text)
end

function getStringWidthGC(text, gc)
	initFontGC(gc)
	return gc:getStringWidth(text)
end

function getStringWidth(text)
	return platform.withGC(getStringWidthGC, text)
end

function initGUI()
	showEditorsBorders = false
	showHLines = true
	scrWidth = platform.window:width()
	scrHeight = platform.window:height()
	if (scrWidth > 0 or scrHeight > 0) then
		theView = View(platform.window)
		sbv = VScrollBar(theView, 0, -1, 5, scrHeight + 1)
		sbv:setHConstraints("right", 0)
		theView:add(sbv)
		fctEditor = MathEditor(theView, 2, border, 50, 30, "")
		-- fctEditor = MathEditor(theView, border, border, 50, 30, "")
		fctEditor:setHConstraints("justify", 1)
		-- fctEditor:setHConstraints("justify", border + scrWidth - sbv.x)
		fctEditor:setVConstraints("bottom", 1)
		fctEditor.editor:setSizeChangeListener(function(editor, w, h)
			return resizeME(editor, w, h)
		end)
		theView:add(fctEditor)
		fctEditor.editor:setText("")
		fctEditor.editor:setBorder(0)
		fctEditor:fixContent()
		sbv:setVConstraints("justify", scrHeight - fctEditor.y + border)
		theView:setFocus(fctEditor)
		inited = true
	end
	toolpalette.enableCopy(true)
	toolpalette.enablePaste(true)
end

function resizeGC(gc)
	scrWidth = platform.window:width()
	scrHeight = platform.window:height()
	if not inited then
		initGUI()
	end
	if inited then
		initFontGC(gc)
		strFullHeight = gc:getStringHeight("H")
		strHeight = strFullHeight - 3
		theView:resize()
		reposME()
		theView:invalidate()
	end
end

function on.resize()
	platform.withGC(resizeGC)
end

forcefocus = true
function on.activate()
	forcefocus = true
end

dispinfos = true
function on.paint(gc)
	if not inited then
		initGUI()
		initFontGC(gc)
		strFullHeight = gc:getStringHeight("H")
		strHeight = strFullHeight - 3
	end
	if inited then
		local obj
		obj = theView:getFocus()
		initFontGC(gc)
		if not obj then theView:setFocus(fctEditor) end
		if (forcefocus) then
			if obj == fctEditor then
				fctEditor.editor:setFocus(true)
				if fctEditor.editor:hasFocus() then forcefocus = false end
			else
				forcefocus = false
			end
		end
		if dispinfos then
			gc:setColorRGB(0)
			gc:setFont("sansserif", "r", 10)
			gc:drawString("Giac CAS engine :", 2, 0, "top")
			if hasGiac then
				gc:setColorRGB(0, 127, 0)
				gc:drawString("OK.", gc:getStringWidth("Giac CAS engine :") + 6, 0, "top")
				gc:setColorRGB(0)
				gc:drawString("Giac (c) B. Parisse/R. De Graeve, license GPL3", 2, 1 * strHeight, "top")
				gc:drawString("Not allowed during exams if", 2, 2 * strHeight, "top")
				gc:drawString("CAS calculators are forbidden!", 2, 3 * strHeight, "top")
				gc:drawString("This is still a beta version.", 2, scrHeight/2-20, "top")
				gc:drawString("Please report any bugs/issues you encounter !", 2, scrHeight/2, "top")
			else
				gc:setColorRGB(255, 0, 0)
				gc:drawString("NO.", gc:getStringWidth("Giac CAS engine :") + 6, 0, "top")
				gc:setColorRGB(0)
				gc:drawString("Make sure to have the .luax file and Ndless installed!", 2, strHeight, "top")
				gc:drawString("Hint: run ndless_installer again", 2, 2 * strHeight, "top")
				gc:setFont("sansserif", "i", 10)
				gc:drawString("Fallback on the Nspire's math engine.", 2, 3 * strHeight + 8, "top")
			end
			gc:setFont("sansserif", "r", fsize)
		end
		theView:paint(gc)
		-- gc:drawString(ts, 2, 0, "top")
		gc:drawRect(0, fctEditor.y - 2, scrWidth, fctEditor.y - 2)
	end
end

--------------------------------------------------------------------- global variables

font = "sansserif"
style = "r"
fsize = 12

scrWidth = 0
scrHeight = 0
inited = false
delim = " ≟ "
border = 3

strHeight = 0
strFullHeight = 0

--------------------------------------------------------------------- global functions

evalstr = false

histME1 = {}
histME2 = {}

function addME(expr, res)
	local mee = MathEditor(theView, border, border, 50, 30, "")
	mee.readOnly = true
	table.insert(histME1, mee)
	mee:setHConstraints("left", border)
	mee.editor:setSizeChangeListener(function(editor, w, h)
		return resizeME(editor, w + 3, h)
	end)
	mee.editor:setExpression("\\0el {" .. expr .. "}", 0)
	mee:fixCursor()
	mee.editor:setReadOnly(true)
	theView:add(mee)

	local mer = MathEditor(theView, border, border, 50, 30, "")
	mer.result = true
	mer.readOnly = true
	table.insert(histME2, mer)
	mer:setHConstraints("right", scrWidth - sbv.x + border)
	mer.editor:setSizeChangeListener(function(editor, w, h)
		return resizeMEpar(editor, w + border, h)
	end)
	mer.editor:setExpression("\\0el {" .. res .. "}", 0)
	mer:fixCursor()
	mer.editor:setReadOnly(true)
	theView:add(mer)
	reposME()
end

limpsuff = "(+)"
limmsuff = "(-)"
function cleanAns1(expr)
	local a, b = expr:find(delim, 1, true)
	local c = 0
	local i = 1
	local l = string.len(expr)
	if a then
		c = expr:find(".", 1, true)
		if not c then c = 0 end
		if c == 0 or c > b then
			expr = expr:sub(1, a - 1)
		else
			expr = expr:sub(b + 1, l)
		end
		return cleanAns1(expr)
	end
	l = string.len(expr)
	a, b = expr:find(limpsuff, 1, true)
	if (not (a)) then
		a, b = expr:find(limmsuff, 1, true)
	else
		a = a - 2
		local texpr = ""
		if (a > 1) then texpr = expr:sub(1, a - 1) end
		if (b < l) then texpr = texpr .. expr:sub(b + 1, l) end
		expr = texpr
	end
	return expr
end

function escapeStr(expr)
	local expr2 = ""
	local l = string.len(expr)
	local c
	for i = 1, l do
		c = expr:sub(i, i)
		if c == "\"" then
			c = "\\\""
		end
		expr2 = expr2 .. c
	end
	return expr2
end

function backSpaceHandler(widget)
	local i = 1
	local f = 0
	local n = mathmax(#histME1, #histME2)
	if (widget ~= fctEditor) then
		while (f == 0 and i <= n) do
			if histME1[i] == widget or histME2[i] == widget then
				f = i
			end
			i = i + 1
		end
	end
	if f > 0 then
		destroyD2Editor(histME1[f].editor)
		destroyD2Editor(histME2[f].editor)
		theView:remove(histME1[f])
		theView:remove(histME2[f])
		table.remove(histME1, f)
		table.remove(histME2, f)
		reposME()
	end
end

function enterHandler(widget)
	local expr, exprkeep
	local svar
	local incerr = "incompatible data type"
	if (widget ~= fctEditor) then
		expr = cleanAns1(widget:getExpression())
		theView:setFocus(fctEditor)
		fctEditor:addString(expr)
	else
		if (fctEditor.editor:getExpression()) then
			expr = fctEditor.editor:getExpression()
			expr = fctEditor:getExpression()
			if (expr and expr ~= "") then
				dispinfos = false
				t1 = timer.getMilliSecCounter()
				res = luagiac.caseval(expr) or "Error"
				t2 = timer.getMilliSecCounter()
				ts  = string.format("Time :  %f" , ( t2 - t1 ) / 1000. )
				fctEditor.editor:setText("")
				fctEditor:fixContent()
				ioffset = 0

				res = " " .. res
				expr = " " .. expr
				expr = expr:gsub("^%s+", " ")
				addME(expr, res)
			end
		end
	end
end

function getParME(editor)
	for i = 1, #histME2 do
		if histME2[i].editor == editor then
			return histME1[i]
		end
	end
	return nil
end

function getME(editor)
	if (fctEditor.editor == editor) then
		return fctEditor
	else
		for i = 1, #histME1 do
			if histME1[i].editor == editor then
				return histME1[i]
			end
		end
		for i = 1, #histME2 do
			if histME2[i].editor == editor then
				return histME2[i]
			end
		end
	end
	return nil
end

function getMEindex(me)
	local ti
	if (fctEditor.editor == me) then
		return 0
	else
		ti = 0
		for i = #histME1, 1, -1 do
			if histME1[i] == me then
				return ti
			end
			ti = ti + 1
		end
		ti = 0
		for i = #histME2, 1, -1 do
			if histME2[i] == me then
				return ti
			end
			ti = ti + 1
		end
	end
	return 0
end

function resizeMEpar(editor, w, h)
	local pare = getParME(editor)
	if pare then
		resizeMElim(editor, w, h, pare.w + pare.dx1 * 2)
	else
		resizeME(editor, w, h)
	end
end

function resizeME(editor, w, h)
	if not editor then return end
	resizeMElim(editor, w, h, scrWidth / 2)
end

function resizeMElim(editor, w, h, lim)
	if not editor then return end
	local met = getME(editor)
	if met then
		met.needw = w
		met.needh = h
		w = mathmax(w, 0)
		w = mathmin(w, scrWidth - met.dx1 * 2)
		h = mathmax(h, strFullHeight + 8)
		if (me ~= fctEditor) then
			w = mathmin(w, (scrWidth - lim) - 2 * met.dx1 + 1)
		end
		met:resize(w, h)
		needcenter = true
		reposME()
		theView:invalidate()
	end
	return editor
end

ioffset = 0
function reposView()
	local focusedME = theView:getFocus()
	if focusedME and focusedME ~= fctEditor then
		local y = focusedME.y
		local h = focusedME.h
		local y0 = fctEditor.y
		local index = getMEindex(focusedME)
		if y < 0 and ioffset < index then
			ioffset = ioffset + 1
			reposME()
			reposView()
		end
		if y + h > y0 and ioffset > index then
			ioffset = ioffset - 1
			reposME()
			reposView()
		end
	end
end

function reposME()
	local h, y, ry, res, i0, beforeh, totalh, visih, h1, h2
	totalh = 0
	beforeh = 0
	visih = 0
	fctEditor.y = scrHeight - fctEditor.h
	theView:repos(fctEditor)
	sbv:setVConstraints("justify", scrHeight - fctEditor.y + border)
	theView:repos(sbv)
	y = fctEditor.y
	i0 = mathmax(#histME1, #histME2)
	for i = i0, 1, -1 do
		h = 0
		h1 = 0
		h2 = 0
		if i <= #histME1 then h1 = mathmax(h1, histME1[i].h) end
		if i <= #histME2 then h2 = mathmax(h2, histME2[i].h) end
		h = mathmax(h1, h2)
		if i0 - i >= ioffset then
			if y >= 0 then
				if y >= h + border then
					visih = visih + h + border
				else
					visih = visih + y
				end
			end
			y = y - h - border
			ry = y
			totalh = totalh + h + border
		else
			ry = scrHeight
			beforeh = beforeh + h + border
			totalh = totalh + h + border
		end
		if i <= #histME1 then
			histME1[i].y = ry
			theView:repos(histME1[i])
			if histME1[i].focus then res = histME1[i] end
		end
		if i <= #histME2 then
			histME2[i].y = ry + mathmax(0, h1 - h2)
			theView:repos(histME2[i])
			if histME2[i].focus then res = histME2[i] end
		end
	end
	if totalh == 0 then
		sbv.pos = 0
		sbv.siz = 100
	else
		sbv.pos = beforeh * 100 / totalh
		sbv.siz = visih * 100 / totalh
	end
	theView:invalidate()
end

function destroyD2Editor(editor)
	if not editor then return end
	editor:setVisible(false)
	editor:move(-10000, -10000)
	editor:resize(1, 1)
	editor = nil
end

function reset()
	for _, v in pairs(theView.widgetList) do
		theView:remove(v)
	end
	for _, e in pairs(histME1) do
		destroyD2Editor(e.editor)
	end
	for _, e in pairs(histME2) do
		destroyD2Editor(e.editor)
	end
	histME1 = {}
	histME2 = {}
	fsize = 10
	applyFontSizeChange()
	platform.window:invalidate()
	myErrorHandler()
end

function myErrorHandler(line, errMsg, callStack, locals)
	if errMsg then print(errMsg) end
	defaultFocus = nil
	theView = nil
	collectgarbage()
	initGUI()
	if errMsg then addME("Script error", errMsg) end
	collectgarbage()
	return true -- let the script continue
end

platform.registerErrorHandler(myErrorHandler)


--------------------------------------------------------------------- toolpalette stuff

function toggleBorders()
	showEditorsBorders = not showEditorsBorders
	on.resize()
end

function set1d()
	 fctEditor.editor:setDisable2DinRT(true)
	on.resize()
end

function set2d()
	 fctEditor.editor:setDisable2DinRT(false)
	on.resize()
end

function toggleHLines()
	showHLines = not showHLines
	on.resize()
end

function applyFontSizeChange()
	fctEditor.editor:setFontSize(fsize)
	for _, e in pairs(histME1) do
		e.editor:setFontSize(fsize)
	end
	for _, e in pairs(histME2) do
		e.editor:setFontSize(fsize)
	end
end

function fontDown()
	fsize = fsize > 6 and (fsize - 1) or fsize
	applyFontSizeChange()
end

function fontUp()
	fsize = fsize < 30 and (fsize + 1) or fsize
	applyFontSizeChange()
end

function menustring( ch )
--	 theView:sendStringToFocus( ch )
	 if fctEditor then fctEditor:addString( ch ) end
end

menu = {
       { "Algebra",
       	 { "factor(expr)",	function() menustring( "factor(" ) end },
       	 { "normal(expr)",	function() menustring( "normal(" ) end },
       	 { "simplify(expr)",	function() menustring( "simplify(" ) end },
       	 { "subst(expr,var,value)",	function() menustring( "subst(" ) end },
       	 { "convert(expr,...)",	function() menustring( "convert(" ) end },
       	 { "solve(expr,var)",	function() menustring( "solve(" ) end },
       	 { "fsolve(expr,var,guess)",	function() menustring( "fsolve(" ) end },
       	 { "rsolve(eq,un)",	function() menustring( "rsolve(" ) end },
       	 { "partfrac(expr)",	function() menustring( "partfrac(" ) end },
       	 { "tcollect(expr)",	function() menustring( "tcollect(" ) end },
       	 { "texpand(expr)",	function() menustring( "texpand(" ) end },
       	 { "cfactor(expr)",	function() menustring( "cfactor(" ) end },
       	 { "cpartfrac(expr)",	function() menustring( "cpartfrac(" ) end },
       	 { "csolve(expr,var)",	function() menustring( "csolve(" ) end },
       },
       { "Calculus",
       	 { "diff(expr,var)",	function() menustring( "diff(" ) end },
       	 { "int(expr,var)",	function() menustring( "int(" ) end },
       	 { "limit(expr,var,value,[1|-1])",	function() menustring( "limit(" ) end },
       	 { "series(expr,var=value,order,[1|-1])",	function() menustring( "series(" ) end },
       	 { "sum(expr,var,min,max)",	function() menustring( "sum(" ) end },
       	 { "desolve(eq,x,y)",	function() menustring( "desolve(" ) end },
       },
       { "Complex and Reals",
       	 { "abs(x)",	function() menustring( "abs(" ) end },
       	 { "floor(x)",	function() menustring( "floor(" ) end },
       	 { "sign(x)",	function() menustring( "sign(" ) end },
       	 { "ceil(x)",	function() menustring( "ceil(" ) end },
       	 { "max(x)",	function() menustring( "max(" ) end },
       	 { "min(x)",	function() menustring( "min(" ) end },
       	 { "round(x[,n])",	function() menustring( "round(" ) end },
       	 { "re(z)",	function() menustring( "re(" ) end },
       	 { "im(z)",	function() menustring( "im(" ) end },
       	 { "conj(z)",	function() menustring( "conj(" ) end },
       	 { "arg(z)",	function() menustring( "arg(" ) end },
       },
       { "Integers",
       	 { "iquo(a,b): euclidean quotient",	function() menustring( "iquo(" ) end },
       	 { "irem(a,b): euclidean remainder",	function() menustring( "irem(" ) end },
       	 { "is_prime(p)",	function() menustring( "is_prime" ) end },
       	 { "nextprime(n)",	function() menustring( "nextprime(" ) end },
       	 { "ifactor(n): factor integer",	function() menustring( "ifactor(" ) end },
       	 { "idivis(n): divisor list",	function() menustring( "idivis(" ) end },
       	 { "iegcd(a,b): a*u+b*v=gcd(a,b)",	function() menustring( "iegcd(" ) end },
       	 { "iabcuv(a,b,c): a*u+b*v=c",	function() menustring( "iabcuv(" ) end },
       	 { "euler(n): Euler indicatrix",	function() menustring( "euler(" ) end },
       	 { "ichrem([a,n],[b,m]): Chinese remainder",	function() menustring( "ichrem(" ) end },
       	 { "powmod(a,m,n): a^m mod n",	function() menustring( "powmod(" ) end },
       },
       { "Linalg",
       	 { "linsolve(",	function() menustring( "linsolve(" ) end },
       	 { "rref(M)",	function() menustring( "rref(" ) end },
       	 { "ref(M)",	function() menustring( "ref(" ) end },
       	 { "ker(M)",	function() menustring( "ker(" ) end },
       	 { "image(M)",	function() menustring( "image(" ) end },
       	 { "det(M)",	function() menustring( "det(" ) end },
       	 { "inv(M)",	function() menustring( "inv(" ) end },
       	 { "bug(M)",	function() menustring( "bug(" ) end },
       	 { "eigenvalues(M)",	function() menustring( "eigenvalues(" ) end },
       	 { "eigenvectors(M)",	function() menustring( "eigenvects(" ) end },
       	 { "jordan(M)",	function() menustring( "jordan(" ) end },
       	 { "matpow(M,n): M^n",	function() menustring( "matpow(" ) end },
       },
       { "Matrix, Vector",
       	 { "dot(v1,v2)",	function() menustring( "dot(" ) end },
       	 { "cross(v1,v2)",	function() menustring( "cross(" ) end },
       	 { "identity(n)",	function() menustring( "identity(" ) end },
       	 { "matrix(n,m,function)",	function() menustring( "matrix(" ) end },
       	 { "randmatrix(n,m,law,[params])",	function() menustring( "randmatrix(" ) end },
       	 { "hilbert(n)",	function() menustring( "hilbert(" ) end },
       	 { "vandermonde(list)",	function() menustring( "vandermonde(" ) end },
       	 { "l1norm(M)",	function() menustring( "l1norm(" ) end },
       	 { "l2norm(M)",	function() menustring( "l2norm(" ) end },
       	 { "linfnorm(M)",	function() menustring( "linfnorm(" ) end },
       	 { "cond(M,1|2|inf)",	function() menustring( "cond(" ) end },
       	 { "lu(M)",	function() menustring( "lu(" ) end },
       	 { "qr(M)",	function() menustring( "qr(" ) end },
       	 { "schur(M)",	function() menustring( "schur(" ) end },
       	 { "svd(M): sing. value dec.",	function() menustring( "svd(" ) end },
       	 { "svl(M): singular values",	function() menustring( "svl(" ) end },
       },
       { "Polynomials",
       	 { "horner(P,x)",	function() menustring( "horner(" ) end },
       	 { "factor(P)",	function() menustring( "factor(" ) end },
       	 { "cfactor(P): complex factorization",	function() menustring( "cfactor(" ) end },
       	 { "proot(P)",	function() menustring( "proot(" ) end },
       	 { "pcoeff(list)",	function() menustring( "pcoeff(" ) end },
       	 { "degree(P,var)",	function() menustring( "degree(" ) end },
       	 { "canonical_form(P,var)",	function() menustring( "canonical_form(" ) end },
       	 { "lagrange(X,Y): interpolation",	function() menustring( "lagrange(" ) end },
       	 { "quorem(A,B): eucl. div.",	function() menustring( "quorem(" ) end },
       	 { "gcd(A,B)",	function() menustring( "gcd(" ) end },
       	 { "egcd(A,B,var): A*U+B*V=gcd(A,B)",	function() menustring( "egcd(" ) end },
       	 { "abcuv(A,B,C,var): A*U+B*V=C",	function() menustring( "abcuv(" ) end },
       	 { "coeff(P,var,n)",	function() menustring( "coeff(" ) end },
       	 { "symb2poly(P,var)",	function() menustring( "symb2poly(" ) end },
       	 { "poly2symb(list,var)",	function() menustring( "poly2symb(" ) end },
       	 { "resultant(A,B,var)",	function() menustring( "resultant(" ) end },
       	 { "gbasis(listpoly,listvar)",	function() menustring( "gbasis(" ) end },
       	 { "cyclotomic(n)",	function() menustring( "cyclotomic(" ) end },
       	 { "hermite(n)",	function() menustring( "hermite(" ) end },
       	 { "tchebyshev1(n)",	function() menustring( "tchebyshev1(" ) end },
       	 { "tchebyshev2(n)",	function() menustring( "tchebyshev2(" ) end },
       	 { "randpoly(n)",	function() menustring( "randpoly(" ) end },
       },
       { "Proba",
       	 { "comb(n,p)",	function() menustring( "comb(" ) end },
       	 { "perm(n,p)",	function() menustring( "perm(" ) end },
       	 { "factorial(n)",	function() menustring( "factorial(" ) end },
       	 { "rand()",	function() menustring( "rand(" ) end },
       	 { "binomial(n,p,k)",	function() menustring( "binomial(" ) end },
       	 { "binomial_cdf(n,p,x)",	function() menustring( "binomial_cdf(" ) end },
       	 { "binomial_icdf(n,p,y)",	function() menustring( "binomial_icdf(" ) end },
       	 { "normald(m,sigma,x)",	function() menustring( "normald(" ) end },
       	 { "normald_cdf(m,sigma,x)",	function() menustring( "normald_cdf(" ) end },
       	 { "normald_icdf(m.sigma,y)",	function() menustring( "normald_icdf(" ) end },
       	 { "poisson(mu,k)",	function() menustring( "poisson(" ) end },
       	 { "exponentiald(mu,x)",	function() menustring( "exponentiald(" ) end },
       	 { "geometric(p,k)",	function() menustring( "geometric(" ) end },
       	 { "chisquared(n,x)",	function() menustring( "chisquared(" ) end },
       	 { "uniformd(a,b,x)",	function() menustring( "uniformd(" ) end },
       	 { "chisquaret(l1,l2): chi2 test",	function() menustring( "chisquaret(" ) end },
       	 { "normalt: Z-test",	function() menustring( "normalt(" ) end },
       	 { "studentt: Student-test",	function() menustring( "studentt(" ) end },
       },
       { "Program",
       	 { ";",	function() menustring( ";\n\r" ) end },
       	 { "function",	function() menustring( "f(x):={ local y; y:=x*x; return y; }" ) end },
       	 { "local",	function() menustring( "local ;\n" ) end },
       	 { "return",	function() menustring( "return ;" ) end },
       	 { "print",	function() menustring( "print(" ) end },
       	 { "if/then",	function() menustring( "if then ; fi;" ) end },
       	 { "if/then/else",	function() menustring( "if then ; else ; fi;" ) end },
       	 { "for",	function() menustring( "for j from 1 to 5 do ; od;" ) end },
       	 { "while",	function() menustring( "while do ; od;" ) end },
       	 { "repeat",	function() menustring( "repeat ; until ;" ) end },
       	 { "break",	function() menustring( "break;" ) end },
       	 { "continue",	function() menustring( "continue;" ) end },
       },
       { "Lists",
       	 { "seq(expr,var,inf,sup)",	function() menustring( "seq(" ) end },
       	 { "size(l)",	function() menustring( "size(" ) end },
       	 { "op(l): list to sequence",	function() menustring( "op(" ) end },
       	 { "apply(f,l) apply function",	function() menustring( "apply(" ) end },
       	 { "append(l,expr)",	function() menustring( "append(" ) end },
       	 { "concat(l1,l2)",	function() menustring( "concat(" ) end },
       	 { "head(l): first element",	function() menustring( "head(" ) end },
       	 { "tail(l)",	function() menustring( "tail(" ) end },
       	 { "sort(l[,function])",	function() menustring( "sort(" ) end },
       	 { "revlist(l)",	function() menustring( "revlist(" ) end },
       	 { "contains(l,expr)",	function() menustring( "contains(" ) end },
       	 { "suppress(l,n)",	function() menustring( "suppress(" ) end },
       	 { "remove(function,l)",	function() menustring( "remove(" ) end },
       },
	{
		"Options",
		{ "save variables",	function() menustring( "write(\"a.tns\",0" ) end },
		{ "read variables",	function() menustring( "eval(read(\"a.tns\"))" ) end },
		"-",
		-- { "1d", set1d },
		-- { "2d", set2d },
		{ "Show/hide editors borders", toggleBorders },
		{ "Show/hide horizontal lines", toggleHLines },
		"-",
		{ "Increase font size", fontUp },
		{ "Decrease font size", fontDown },
		"-",
		{ "Restart CAS",	function() menustring( "restart;" ) end },
		{ "Clear history", reset }
	}
}
toolpalette.register(menu)
