
function this.cameraScript(--[[CameraTrackingData*]] ctd)
  if (this:count() == 0) then
    return;
  end
  
  local N_shift = { x = 0, y = 0, z = 0 };
  
  local target = this:getFirst();
  while (target ~= nil) do
    N_shift.x = N_shift.x + target.P_center_Trfm.x;
	N_shift.y = N_shift.y + target.P_center_Trfm.y;
	N_shift.z = N_shift.z + target.P_center_Trfm.z;
	
	target = this:getNext();
  end
  
  this.P_destination.x = N_shift.x;
  this.P_destination.y = N_shift.y;
  this.P_destination.z = N_shift.z;
end

vec3 = {};
plane = {};

do -- vec3 && plane

function vec3.Add(...)
  local res = select(1,...);
  for n=2,select('#',...) do
    local e = select(n,...);
    res = { x = res.x + e.x, y = res.y + e.y, z = res.z + e.z };
  end
  return res;
end

function vec3.FromPoints(P1, P2)
  return { x = P1.x - P2.x, y = P1.y - P2.y, z = P1.z - P2.z };
end

function vec3.Invert(N_vec)
  return { x = -N_vec.x, y = -N_vec.y, z = -N_vec.z };
end

function vec3.MulScalar(N_vec1, scalar)
  return { x = N_vec1.x * scalar, y = N_vec1.y * scalar, z = N_vec1.z * scalar };
end

function vec3.DotProduct(N_vec1, N_vec2)
  return N_vec1.x * N_vec2.x + N_vec1.y * N_vec2.y + N_vec1.z * N_vec2.z
end

function vec3.CrossProduct(N_vec1, N_vec2)
  return { x = N_vec1.y * N_vec2.z - N_vec1.z * N_vec2.y,
           y = N_vec1.z * N_vec2.x - N_vec1.x * N_vec2.z,
           z = N_vec1.x * N_vec2.y - N_vec1.y * N_vec2.x };
end

function vec3.Normalize(N_vec)
  local len = math.sqrt(vec3.DotProduct(N_vec, N_vec));
  if (len ~= 0 and len ~= 1) then
      return vec3.MulScalar(N_vec, 1 / len);
  end
  return N_vec;
end

function plane.DistanceToPoint(PN_plane, P_point)
  return vec3.DotProduct(PN_plane, P_point) + PN_plane.w;
end

end

local cam = {};

do -- helper functions

function cam.P_MinMax_Get( P_center, S_radius ) -- returns P_min, P_max
	local P_min, P_max = {}, {};
	P_min.x, P_max.x = P_center.x - S_radius, P_center.x + S_radius;
	P_min.y, P_max.y = P_center.y - S_radius, P_center.y + S_radius;
	P_min.z, P_max.z = P_center.z - S_radius, P_center.z + S_radius;
	return P_min, P_max;
end

function cam.ComputeSpan(P_center, S_radius, N_axis) -- returns S_min, S_max
  local P_middle = vec3.DotProduct(N_axis, P_center);
  return P_middle - S_radius, P_middle + S_radius;
end

end

function this.EyeSeeAll_Center2(self, --[[CameraTrackingData*]] ctd)

  if (this:count() == 0) then return; end
  
  local P_min_d, P_max_d, P_min, P_max, P_min_t, P_max_t, P_tmp;

  -- Find Bounding Box
  local target = this:getFirst();
  P_min,   P_max   = target.P_center_Trfm, target.P_center_Trfm;
  P_min_d, P_max_d = cam.P_MinMax_Get( target.P_center_Trfm, target.S_radius );
  
  target = this:getNext();
  while (target ~= nil) do
    P_min_t, P_max_t = cam.P_MinMax_Get( target.P_center_Trfm, target.S_radius );
    
    if (P_min_t.x < P_min_d.x) then P_min_d.x = P_min_t.x; end
	if (P_max_t.x > P_max_d.x) then P_max_d.x = P_max_t.x; end
	if (P_min_t.y < P_min_d.y) then P_min_d.y = P_min_t.y; end
	if (P_max_t.y > P_max_d.y) then P_max_d.y = P_max_t.y; end
	if (P_min_t.z < P_min_d.z) then P_min_d.z = P_min_t.z; end
	if (P_max_t.z > P_max_d.z) then P_max_d.z = P_max_t.z; end

	P_tmp = target.P_center_Trfm;
	if     (P_tmp.x < P_min.x) then P_min.x = P_tmp.x;
	elseif (P_tmp.x > P_max.x) then P_max.x = P_tmp.x; end
	if     (P_tmp.y < P_min.y) then P_min.y = P_tmp.y;
	elseif (P_tmp.y > P_max.y) then P_max.y = P_tmp.y; end
	if     (P_tmp.z < P_min.z) then P_min.z = P_tmp.z;
	elseif (P_tmp.z > P_max.z) then P_max.z = P_tmp.z; end
	
    target = this:getNext();
  end

  local P_center = vec3.MulScalar( vec3.Add(P_min, P_max), 0.5 );
  --P_center.x, P_center.y, P_center.z = (P_min.x + P_max.x) * 0.5, (P_min.y + P_max.y) * 0.5, (P_min.z + P_max.z) * 0.5;

  -- Find a diagonal with the longest span covering
  local NW_diagonal = {};
  NW_diagonal[1] = { x = P_max.x - P_min.x, y = P_max.y - P_min.y, z = P_max.z - P_min.z };
  NW_diagonal[2] = { x = P_min.x - P_max.x, y = P_max.y - P_min.y, z = P_max.z - P_min.z };
  NW_diagonal[3] = { x = P_max.x - P_min.x, y = P_min.y - P_max.y, z = P_max.z - P_min.z };
  NW_diagonal[4] = { x = P_min.x - P_max.x, y = P_min.y - P_max.y, z = P_max.z - P_min.z };
    
  local S_min, S_max;
  local S_span_min, S_span_max =
          { 1000000.0, 1000000.0, 1000000.0, 1000000.0 },
          { -1000000.0, -1000000.0, -1000000.0, -1000000.0 };

  target = this:getFirst();
  while (target ~= nil) do
    for i = 1, 4 do
      S_min, S_max = cam.ComputeSpan(target.P_center_Trfm, target.S_radius, NW_diagonal[i]);
      if (S_min < S_span_min[i]) then S_span_min[i] = S_min; end
      if (S_max > S_span_max[i]) then S_span_max[i] = S_max; end
    end
    target = this:getNext();
  end
  
  S_max = S_span_max[1] - S_span_min[1];
  local I_max = 1;
  for i = 2, 4 do
    S_min = S_span_max[i] - S_span_min[i];
    if (S_min > S_max) then
      S_max = S_min;
      I_max = i;
    end
  end

  -- Find vector to new eye
  local N_up;
  if (NW_diagonal[I_max].x == 0 and NW_diagonal[I_max].y == 0) then
    N_up = { x = 1, y = 0, z = 0 };
  else
    N_up = { x = 0, y = 0, z = 1 };
  end

  local N_front = vec3.Normalize(vec3.CrossProduct(N_up, NW_diagonal[I_max]));
  
  local PN_screen = N_front; PN_screen.w = - vec3.DotProduct(N_front, P_center);

  if (plane.DistanceToPoint(PN_screen, ctd.P_current) < 0) then
    vec3.Invert(N_front);
    vec3.Invert(PN_screen);
  end
  local N_side = vec3.CrossProduct(N_up, N_front);

  -- Find span in the screen direction
  S_span_max[1] = -1000000.0;
  target = this:getFirst();
  while (target ~= nil) do
    S_min, S_max = cam.ComputeSpan(target.P_center_Trfm, target.S_radius, N_front);
    if (S_max > S_span_max[1]) then S_span_max[1] = S_max; end
    target = this:getNext();
  end
  
  S_min = vec3.DotProduct(N_front, P_center);
  
  local S_front = S_span_max[1] - S_min;
  local S_up    = math.abs(vec3.DotProduct(N_up,   vec3.FromPoints(P_max_d, P_min_d))) * 0.5;
  local S_side  = math.abs(vec3.DotProduct(N_side, NW_diagonal[I_max])) * 0.5;

  if (ctd.Projection == "perspective") then
    S_up = math.max(S_up, S_side/ctd.Aspect) * 1.5;
    local S_aux = S_up / math.tan( math.rad(ctd.PerspAngle) * 0.5 );
    this.P_destination = vec3.Add( P_center, vec3.MulScalar( N_front, S_front + S_aux) );
  else
    S_up = math.max(S_up * ctd.Aspect, S_side);
    this.P_destination = vec3.Add( P_center, vec3.MulScalar( N_front, S_up) );
  end
    
  this.P_destination = 
    vec3.Add( this.P_destination,
              vec3.MulScalar(N_front, this.NW_destination_shift.y),
              vec3.MulScalar(N_side,  this.NW_destination_shift.x),
              vec3.MulScalar(N_up,    this.NW_destination_shift.z) );
end
