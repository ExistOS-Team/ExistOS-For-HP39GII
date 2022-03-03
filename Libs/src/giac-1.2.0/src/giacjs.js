// file: giacjs.js

// ----- Object creation -----

print("Creating some objects:");
ct = new giacjs.context();
c = new giacjs.gen(100);
d = new giacjs.gen(10);
c = giacjs.pow(c,d,ct);
d=giacjs.ifactor(c,ct);
print(d.print(ct));
s ="1/(x^4-1)";
d=giacjs.genfromstring(s);
print(d.print(ct));
d=giacjs._eval(d,ct);
print(giacjs._factor(d,ct).print(ct));
print(giacjs._integrate(d,ct).print(ct));